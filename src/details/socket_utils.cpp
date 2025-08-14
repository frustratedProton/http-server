#include "socket_utils.hpp"
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int create_server_socket(int port) {
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    std::cerr << "Failed to create socket\n";
    exit(1);
  }
  int reuse = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);

  if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    std::cerr << "Bind failed\n";
    exit(1);
  }
  return server_fd;
}

int accept_client(int server_fd) {
  sockaddr_in client_addr{};
  socklen_t client_len = sizeof(client_addr);
  return accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
}
