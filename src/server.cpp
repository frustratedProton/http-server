#include "server.hpp"
#include "details/socket_utils.hpp"
#include "request_handler.hpp"
#include <iostream>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>

Server::Server(int port, const std::string &filesDir)
    : port(port), files_directory(filesDir) {}

void Server::run() {
  server_fd = create_server_socket(port);
  startListening();
  acceptClients();
}

void Server::startListening() {
  if (listen(server_fd, 5) != 0) {
    std::cerr << "Could not listen on port " << port << "\n";
    exit(1);
  }
  std::cout << "Server running on port " << port << "\n";
}

void Server::acceptClients() {
  while (true) {
    int client_fd = accept_client(server_fd);
    if (client_fd < 0)
      continue;

    int *pClient = new int(client_fd);
    pthread_t tid;
    if (pthread_create(&tid, nullptr, RequestHandler::handleClientThread,
                       pClient) != 0) {
      std::cerr << "Failed to create thread\n";
      close(client_fd);
      delete pClient;
    } else {
      pthread_detach(tid);
    }
  }
}