#include "server.hpp"
#include "details/socket_utils.hpp"
#include "details/thread_pool.hpp"
#include "request_handler.hpp"
#include <iostream>
#include <sys/socket.h>
#include <thread>

Server::Server(int port, const std::string &filesDir)
    : port(port), files_directory(filesDir) {}

void Server::run() {
  server_fd = create_server_socket(port);
  startListening();
  acceptClients();
}

void Server::startListening() {
  if (listen(server_fd, SOMAXCONN) != 0) { 
    std::cerr << "Could not listen on port " << port << "\n";
    exit(1);
  }
  std::cout << "Server running on port " << port << "\n";
}

void Server::acceptClients() {
  size_t num_threads = std::thread::hardware_concurrency() * 2;

  ThreadPool pool(num_threads,
                  [](int fd) { RequestHandler::handleClient(fd); });

  std::cout << "Thread pool started with " << num_threads << " workers\n";

  while (true) {
    int client_fd = accept_client(server_fd);
    if (client_fd < 0)
      continue;
    pool.submit(client_fd);
  }
}