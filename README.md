# C++ HTTP Server

This project is a simple **HTTP/1.1 server** written in **C++**. The goal is to implement a fully-functional HTTP server that can handle multiple client connections, parse HTTP requests, and serve responses accordingly.

---

## Getting Started

### Prerequisites

* **C++ Compiler** (g++ or clang++)
* **CMake** (for building the project)
* **VCPKG** (for dependency management, if needed)

### Setup

1. Clone the repository:

   ```bash
   git clone https://github.com/frustratedProton/http-server
   cd http-server
   ```

2. **Build the project** using the provided `runner.sh` script. This will set up everything locally for you:

   ```bash
   ./runner.sh
   ```

   This script will:

   * Run `cmake` to configure the project.
   * Build the project using `cmake --build`.
   * Ensure that the necessary dependencies are correctly managed.

---

## How to Run

1. Once the project is built, you can run the server locally using the same `runner.sh` script with an optional   `--directory` flag:

   ```bash
   ./runner.sh [--directory <absolute_path>]
   ```

* The optional `--directory` flag specifies the absolute path to serve files from when clients request `/files/{filename}`.
* If omitted, requests to `/files/{filename}` will respond with `400 Bad Request`.

## Testing the Server

You can test the server endpoints with HTTP clients such as `curl`:

* Echo endpoint:

  ```bash
  curl http://localhost:8080/echo/your_message
  ```

* User-Agent endpoint:

  ```bash
  curl -i -H "User-Agent: CustomAgent" http://localhost:8080/user-agent
  ```

* Files endpoint (requires `--directory` flag to be set):

  ```bash
  curl http://localhost:8080/files/filename
  ```

---

## Additional Resources

* [HTTP Request Syntax](https://www.rfc-editor.org/rfc/rfc9110.html)
* [Learn More About HTTP](https://en.wikipedia.org/wiki/Hypertext_Transfer_Protocol)
* [Learn About HTTP Response](https://developer.mozilla.org/en-US/docs/Web/HTTP/Guides/Messages#http_responses)