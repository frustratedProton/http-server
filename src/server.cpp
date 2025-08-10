#include <cstdlib>
#include <iostream>
#include <netinet/in.h>
#include <ostream>
#include <sys/socket.h>
#include <unistd.h>

int main() {
  // create a socket
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) {
    std::cerr << "Failed to create socket! \n" << std::endl;
    return 1;
  }

  // setup server addr
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr =
      INADDR_ANY; // Bind to any available network interface
  server_addr.sin_port = htons(8080);

  // bind the socket to the addr and port
  if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) !=
      0) {
    std::cerr << "Failed to bind to port 8080! \n" << std::endl;
    return 1;
  }

  std::cout << "Successfully bound to port 8080! \n" << std::endl;

  // close the socket
  close(socket_fd);

  return EXIT_SUCCESS;
}
