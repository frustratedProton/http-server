#include "request_handler.hpp"
#include "server.hpp"
#include <csignal>
#include <iostream>

int main(int argc, char *argv[]) {
  signal(SIGPIPE, SIG_IGN);

  std::string filesDir = "";
  if (argc == 3 && std::string(argv[1]) == "--directory") {
    filesDir = argv[2];
  }

  RequestHandler::files_directory = filesDir;

  Server server(8080, filesDir);
  server.run();
  return 0;
}
