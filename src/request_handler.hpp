#pragma once
#include <string>

class RequestHandler {
public:
  static void *handleClientThread(void *arg);
  static std::string files_directory;

private:
  static void handleRequest(int client_fd, const std::string &request);
};
