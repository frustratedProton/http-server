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

1. Once the project is built, you can run the server locally using the same `runner.sh` script:

   ```bash
   ./runner.sh
   ```

2. To test the server, use any HTTP client (browser, `curl`, Postman, etc.) and make requests to `localhost` at the appropriate port (8080 in my implementation).

---

## Additional Resources

* [HTTP Request Syntax](https://www.rfc-editor.org/rfc/rfc9110.html)
* [Learn More About HTTP](https://en.wikipedia.org/wiki/Hypertext_Transfer_Protocol)
