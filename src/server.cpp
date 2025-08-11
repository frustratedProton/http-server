#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <ostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main() {
  // Create a socket
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    std::cerr << "Failed to create socket! \n" << std::endl;
    return 1;
  }

  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) <
      0) {
    std::cerr << "setsockopt failed! \n" << std::endl;
    return 1;
  }

  // Setup server address
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr =
      INADDR_ANY; // Bind to any available network interface
  server_addr.sin_port = htons(8080);

  // Bind the socket to the addr and port
  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) !=
      0) {
    std::cerr << "Failed to bind to port 8080! \n" << std::endl;
    return 1;
  }

  // Listen for incoming connections
  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    std::cerr << "Could not listen to incoming connections! \n" << std::endl;
    return 1;
  }

  std::cout << "Server running. Waiting for clients to connect...\n"
            << std::endl;

  // Infinite loop to keep the server running indefinitely
  while (true) {
    // Accept an incoming connection
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fd =
        accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_fd < 0) {
      std::cerr << "Failed to accept connection! \n" << std::endl;
      continue; // Continue listening for new connections even if accept() fails
    }

    std::cout << "Client connected! \n" << std::endl;

    // recieve req from client
    char buffer[BUFFER_SIZE];
    ssize_t bytes_recv = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_recv < 0) {
      std::cerr << "Failed to receive data! \n" << std::endl;
      close(client_fd);
      continue;
    }

    buffer[bytes_recv] = '\0'; // null termination for valid c style str

    // debugging
    // std::cout << "Received request:\n" << buffer << std::endl;

    char method[16], path[256], version[16];
    if (sscanf(buffer, "%15s %255s %15s", method, path, version) != 3) {
      std::cerr << "Invalid HTTP request format.\n";
      close(client_fd);
      continue;
    }

    std::cout << "HTTP Method: " << method << ", Path: " << path
              << ", Version: " << version << "\n";

    // check if method is GET and path = '/echo/{str}'
    const char *http_response = nullptr;
    if (strcmp(method, "GET") == 0 && strncmp(path, "/echo/", 6) == 0) {

      // extract the str after '/echo/'
      const char *echo_str = path + 6;

      char resp_header[BUFFER_SIZE];
      int body_length = strlen(echo_str);
      snprintf(resp_header, sizeof(resp_header),
               "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: "
               "%d\r\n\r\n",
               body_length);

      char resp_body[BUFFER_SIZE];
      snprintf(resp_body, sizeof(resp_body), "%s", echo_str);

      // send the resp_header and body back to client
      ssize_t bytes_send = send(client_fd, resp_header, strlen(resp_header), 0);
      if (bytes_send < 0) {
        std::cerr << "Failed to send response header! \n" << std::endl;
      }

      bytes_send = send(client_fd, resp_body, strlen(resp_body), 0);
      if (bytes_send < 0) {
        std::cerr << "Failed to send response body! \n" << std::endl;
      }

      std::cout << "Sent HTTP response: " << resp_header << resp_body << "\n";
    } else {
      const char *http_404_resp = "HTTP/1.1 404 Not Found\r\n\r\n";
      ssize_t bytes_send =
          send(client_fd, http_404_resp, strlen(http_404_resp), 0);
      if (bytes_send < 0) {
        std::cerr << "Failed to send 404 response! \n" << std::endl;
      }
      std::cout << "Sent HTTP response: " << http_404_resp << "\n";
    }

    // Close the client socket after the response
    close(client_fd);
  }

  // The server will run indefinitely,
  // so this line won't be reached unless terminated manually
  close(server_fd);
  return EXIT_SUCCESS;
}
