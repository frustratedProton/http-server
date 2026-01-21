#pragma once
#include "http_parser.hpp"
#include <string>

class RequestHandler {
public:
  static void handleClient(int client_fd);
  static void *handleClientThread(void *arg);
  static std::string files_directory;

private:
  static void handleRequest(int client_fd, const HttpRequest &request);
};
