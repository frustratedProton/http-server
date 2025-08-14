#include "request_handler.hpp" 
#include "server.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
  std::string filesDir = "";
  if (argc == 3 && std::string(argv[1]) == "--directory") {
    filesDir = argv[2];
  }

  RequestHandler::files_directory = filesDir; 
  
  Server server(8080, filesDir);
  server.run();
  return 0;
}
