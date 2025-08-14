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

void *RequestHandler::handleClientThread(void *arg) {
  int client_fd = *(int *)arg;
  delete (int *)arg;

  char buffer[1024];
  ssize_t bytes_recv = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
  if (bytes_recv <= 0) {
    close(client_fd);
    return nullptr;
  }
  buffer[bytes_recv] = '\0';

  handleRequest(client_fd, std::string(buffer));
  close(client_fd);
  return nullptr;
}

void RequestHandler::handleRequest(int client_fd, const std::string &request) {
  HttpRequest req = HttpParser::parse(request);

  std::ostringstream resp;

  // /echo/<msg>
  if (req.method == "GET" && req.path.rfind("/echo/", 0) == 0) {
    std::string body = req.path.substr(6);

    bool clientAccpetGzip = false;
    auto it = req.headers.find("Accept-Encoding");
    if (it != req.headers.end()) {
      std::string encodingDirective = it->second;
      std::transform(encodingDirective.begin(), encodingDirective.end(),
                     encodingDirective.begin(), ::tolower);
      if (encodingDirective.find("gzip") != std::string::npos) {
        clientAccpetGzip = true;
      }
    }

    std::string outBody = body;
    if (clientAccpetGzip) {
      outBody = gzipCompress(body);
    }

    std::ostringstream resp;
    resp << "HTTP/1.1 200 OK\r\n";
    resp << "Content-Type: text/plain\r\n";
    if (clientAccpetGzip) {
      resp << "Content-Encoding: gzip\r\n";
    }
    resp << "Content-Length: " << outBody.size() << "\r\n\r\n";

    send(client_fd, resp.str().c_str(), resp.str().size(), 0);
    send(client_fd, outBody.data(), outBody.size(), 0);
    return;
  }

  // /user-agent
  if (req.method == "GET" && req.path == "/user-agent") {
    std::string ua =
        req.headers.count("User-Agent") ? req.headers.at("User-Agent") : "";
    resp << "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: "
         << ua.size() << "\r\n\r\n"
         << ua;
    send(client_fd, resp.str().c_str(), resp.str().size(), 0);
    return;
  }

  // /files/<filename> GET
  if (req.method == "GET" && req.path.rfind("/files/", 0) == 0) {
    std::string filename = req.path.substr(7);
    std::ifstream file(files_directory + "/" + filename, std::ios::binary);
    if (!file) {
      std::string notFound = "File not found";
      resp << "HTTP/1.1 404 Not Found\r\nContent-Type: "
              "text/plain\r\nContent-Length: "
           << notFound.size() << "\r\n\r\n"
           << notFound;
    } else {
      std::ostringstream filebuf;
      filebuf << file.rdbuf();
      std::string content = filebuf.str();
      resp << "HTTP/1.1 200 OK\r\nContent-Type: "
              "application/octet-stream\r\nContent-Length: "
           << content.size() << "\r\n\r\n"
           << content;
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
      resp << "HTTP/1.1 500 Internal Server Error\r\nContent-Type: "
              "text/plain\r\nContent-Length: "
           << err.size() << "\r\n\r\n"
           << err;
    } else {
      file << req.body;
      std::string ok = "File created";
      resp << "HTTP/1.1 201 Created\r\nContent-Type: "
              "text/plain\r\nContent-Length: "
           << ok.size() << "\r\n\r\n"
           << ok;
    }
    send(client_fd, resp.str().c_str(), resp.str().size(), 0);
    return;
  }

  // Fallback 404
  std::string notFound = "Not found";
  resp << "HTTP/1.1 404 Not Found\r\nContent-Type: "
          "text/plain\r\nContent-Length: "
       << notFound.size() << "\r\n\r\n"
       << notFound;
  send(client_fd, resp.str().c_str(), resp.str().size(), 0);
}
