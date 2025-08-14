#include "http_parser.hpp"
#include <sstream>

HttpRequest HttpParser::parse(const std::string &raw) {
  HttpRequest req;
  std::istringstream stream(raw);
  stream >> req.method >> req.path >> req.version;

  std::string line;
  std::getline(stream, line); // consume rest of first line

  while (std::getline(stream, line) && line != "\r") {
    auto colon = line.find(":");
    if (colon != std::string::npos) {
      std::string key = line.substr(0, colon);
      std::string value = line.substr(colon + 2);
      if (!value.empty() && value.back() == '\r')
        value.pop_back();
      req.headers[key] = value;
    }
  }

  if (req.headers.count("Content-Length")) {
    int len = std::stoi(req.headers["Content-Length"]);
    req.body.resize(len);
    stream.read(&req.body[0], len);
  }
  return req;
}
