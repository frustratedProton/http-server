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

1. Once built, run the server locally (optionally specifying a files directory):

   ```bash
   ./runner.sh --directory /absolute/path/to/files
   ```

2. The server listens on port 8080 by default.

---

## Supported Endpoints

### GET /echo/{string}

Responds with the text following `/echo/` in the path. If the request includes `Accept-Encoding: gzip`, the response will be gzip-compressed.

Example (plain text):

```bash
curl http://localhost:8080/echo/hello
```

Response:

```
hello
```

Example (gzip):

```bash
curl -v -H "Accept-Encoding: gzip" http://localhost:8080/echo/hello --output -
```

Response headers:

```
HTTP/1.1 200 OK
Content-Type: text/plain
Content-Encoding: gzip
Content-Length: 23
```

Use `--compressed` to see decompressed output:

```bash
curl --compressed http://localhost:8080/echo/hello
```

Output:

```
hello
```

---

### GET /user-agent

Responds with the value of the `User-Agent` header sent by the client.

Example:

```bash
curl -i http://localhost:8080/user-agent
```

---

### GET /files/{filename}

Returns the contents of the file `{filename}` located in the specified directory.

Example:

```bash
curl http://localhost:8080/files/example.txt
```

---

### POST /files/{filename}

Creates or overwrites a file named `{filename}` in the files directory with the request body.

* The request must include:

  * `Content-Type: application/octet-stream`
  * `Content-Length` header specifying the body size.
  * The request body containing the file contents.

Example:

```bash
curl -v -X POST http://localhost:8080/files/myfile.txt \
     -H "Content-Type: application/octet-stream" \
     --data "This is the file content."
```

Response:

```
HTTP/1.1 201 Created
```

The file will be saved as `/absolute/path/to/files/myfile.txt`.

---

## Additional Resources

* [HTTP Request Syntax](https://www.rfc-editor.org/rfc/rfc9110.html)
* [Learn More About HTTP](https://en.wikipedia.org/wiki/Hypertext_Transfer_Protocol)
* [Learn About HTTP Response](https://developer.mozilla.org/en-US/docs/Web/HTTP/Guides/Messages#http_responses)
