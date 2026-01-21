#include "request_handler.hpp"
#include "details/compression.hpp"
#include "http_parser.hpp"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

std::string RequestHandler::files_directory = "";

void RequestHandler::handleClient(int client_fd) {
  std::string buffer;
  char temp[4096];

  while (true) {
    ssize_t bytes_recv = recv(client_fd, temp, sizeof(temp), 0);
    if (bytes_recv <= 0)
      break;

    buffer.append(temp, bytes_recv);

    while (true) {
      size_t header_end = buffer.find("\r\n\r\n");
      if (header_end == std::string::npos)
        break;

      std::string header_str = buffer.substr(0, header_end + 4);
      HttpRequest req = HttpParser::parse(header_str);

      size_t total_len = header_end + 4;
      if (req.method == "POST" && req.headers.count("Content-Length")) {
        int content_len = std::stoi(req.headers["Content-Length"]);
        if (buffer.size() < total_len + content_len)
          break;

        req.body = buffer.substr(total_len, content_len);
        total_len += content_len;
      }

      handleRequest(client_fd, req);
      buffer.erase(0, total_len);

      auto it = req.headers.find("Connection");
      if (it != req.headers.end()) {
        std::string val = it->second;
        std::transform(val.begin(), val.end(), val.begin(), ::tolower);
        if (val == "close") {
          return; // caller will close
        }
      }
    }
  }
}

void *RequestHandler::handleClientThread(void *arg) {
  int client_fd = *(int *)arg;
  delete (int *)arg;

  std::string buffer;
  char temp[4096];
  while (true) {
    ssize_t bytes_recv = recv(client_fd, temp, sizeof(temp), 0);
    if (bytes_recv <= 0)
      break;

    buffer.append(temp, bytes_recv);

    while (true) {
      size_t header_end = buffer.find("\r\n\r\n");
      if (header_end == std::string::npos)
        break;

      std::string header_str = buffer.substr(0, header_end + 4);
      HttpRequest req = HttpParser::parse(header_str);

      // Determine if body is complete (for POST requests)
      size_t total_len = header_end + 4;
      if (req.method == "POST" && req.headers.count("Content-Length")) {
        int content_len = std::stoi(req.headers["Content-Length"]);
        if (buffer.size() < total_len + content_len)
          break;

        req.body = buffer.substr(total_len, content_len);
        total_len += content_len;
      }

      handleRequest(client_fd, req);

      buffer.erase(0, total_len);

      auto it = req.headers.find("Connection");
      if (it != req.headers.end()) {
        std::string val = it->second;
        std::transform(val.begin(), val.end(), val.begin(), ::tolower);
        if (val == "close") {
          close(client_fd);
          return nullptr;
        }
      }
    }
  }

  close(client_fd);
  return nullptr;
}

void RequestHandler::handleRequest(int client_fd, const HttpRequest &req) {
  std::ostringstream resp;

  // Check if client requested to close the connection
  bool shouldClose = false;
  auto conn_it = req.headers.find("Connection");
  if (conn_it != req.headers.end()) {
    std::string val = conn_it->second;
    std::transform(val.begin(), val.end(), val.begin(), ::tolower);
    if (val == "close") {
      shouldClose = true;
    }
  }

  // /echo/<msg>
  if (req.method == "GET" && req.path.rfind("/echo/", 0) == 0) {
    std::string body = req.path.substr(6);

    bool clientAcceptGzip = false;
    auto it = req.headers.find("Accept-Encoding");
    if (it != req.headers.end()) {
      std::string encodingDirective = it->second;
      std::transform(encodingDirective.begin(), encodingDirective.end(),
                     encodingDirective.begin(), ::tolower);
      if (encodingDirective.find("gzip") != std::string::npos) {
        clientAcceptGzip = true;
      }
    }

    std::string outBody = body;
    if (clientAcceptGzip)
      outBody = gzipCompress(body);

    resp << "HTTP/1.1 200 OK\r\n";
    resp << "Content-Type: text/plain\r\n";
    if (clientAcceptGzip)
      resp << "Content-Encoding: gzip\r\n";
    resp << (shouldClose ? "Connection: close\r\n"
                         : "Connection: keep-alive\r\n");
    resp << "Content-Length: " << outBody.size() << "\r\n\r\n";

    send(client_fd, resp.str().c_str(), resp.str().size(), 0);
    send(client_fd, outBody.data(), outBody.size(), 0);
    return;
  }

  // /user-agent
  if (req.method == "GET" && req.path == "/user-agent") {
    std::string ua =
        req.headers.count("User-Agent") ? req.headers.at("User-Agent") : "";

    resp << "HTTP/1.1 200 OK\r\n";
    resp << "Content-Type: text/plain\r\n";
    resp << (shouldClose ? "Connection: close\r\n"
                         : "Connection: keep-alive\r\n");
    resp << "Content-Length: " << ua.size() << "\r\n\r\n";
    resp << ua;

    send(client_fd, resp.str().c_str(), resp.str().size(), 0);
    return;
  }

  // /files/<filename> GET
  if (req.method == "GET" && req.path.rfind("/files/", 0) == 0) {
    std::string filename = req.path.substr(7);
    std::ifstream file(files_directory + "/" + filename, std::ios::binary);

    if (!file) {
      std::string notFound = "File not found";
      resp << "HTTP/1.1 404 Not Found\r\n";
      resp << "Content-Type: text/plain\r\n";
      resp << (shouldClose ? "Connection: close\r\n"
                           : "Connection: keep-alive\r\n");
      resp << "Content-Length: " << notFound.size() << "\r\n\r\n";
      resp << notFound;
    } else {
      std::ostringstream filebuf;
      filebuf << file.rdbuf();
      std::string content = filebuf.str();

      resp << "HTTP/1.1 200 OK\r\n";
      resp << "Content-Type: application/octet-stream\r\n";
      resp << (shouldClose ? "Connection: close\r\n"
                           : "Connection: keep-alive\r\n");
      resp << "Content-Length: " << content.size() << "\r\n\r\n";
      resp << content;
    }

    send(client_fd, resp.str().c_str(), resp.str().size(), 0);
    return;
  }

  // /files/<filename> POST
  if (req.method == "POST" && req.path.rfind("/files/", 0) == 0) {
    std::string filename = req.path.substr(7);
    std::ofstream file(files_directory + "/" + filename, std::ios::binary);

    if (!file) {
      std::string err = "Unable to write file";
      resp << "HTTP/1.1 500 Internal Server Error\r\n";
      resp << "Content-Type: text/plain\r\n";
      resp << (shouldClose ? "Connection: close\r\n"
                           : "Connection: keep-alive\r\n");
      resp << "Content-Length: " << err.size() << "\r\n\r\n";
      resp << err;
    } else {
      file << req.body;
      std::string ok = "File created";
      resp << "HTTP/1.1 201 Created\r\n";
      resp << "Content-Type: text/plain\r\n";
      resp << (shouldClose ? "Connection: close\r\n"
                           : "Connection: keep-alive\r\n");
      resp << "Content-Length: " << ok.size() << "\r\n\r\n";
      resp << ok;
    }

    send(client_fd, resp.str().c_str(), resp.str().size(), 0);
    return;
  }

  // Fallback 404
  std::string notFound = "Not found";
  resp << "HTTP/1.1 404 Not Found\r\n";
  resp << "Content-Type: text/plain\r\n";
  resp << (shouldClose ? "Connection: close\r\n"
                       : "Connection: keep-alive\r\n");
  resp << "Content-Length: " << notFound.size() << "\r\n\r\n";
  resp << notFound;

  send(client_fd, resp.str().c_str(), resp.str().size(), 0);
}
