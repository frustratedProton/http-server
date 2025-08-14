#pragma once
#include <string>

class Server {
public:
  Server(int port, const std::string &filesDir = "");
  void run();

private:
  int server_fd;
  int port;
  std::string files_directory;

  void startListening();
  void acceptClients();
};