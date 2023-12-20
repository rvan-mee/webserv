# Project WebServ

Welcome to Project WebServ! This project is part of the 42 curriculum and focuses on creating a basic web server from scratch. The goal is to gain a deep understanding of how web servers function by implementing key features such as handling HTTP requests, parsing configuration files, and serving static files.

## Getting Started

To get started with Project WebServ, follow these steps:

1. Clone the repository
2. Build the project using `make`
3. Run the web server `./webserv <configuration-file>`

## Features

- **HTTP/1.1 Support:** The web server supports the HTTP/1.1 protocol for handling client requests.
- **Methods:** Implemented support for the HTTP methods GET, POST, and DELETE.
- **Configurable:** Easily configure the web server using a specified configuration file.
- **Static File Serving:** Serve static files from the configured document root.
- **File Uploading:** Upload files using the POST method.
- **Error Handling:** Handle common HTTP errors and return appropriate status codes.
- **CGI:** Implemented CGI support using python.
- **IO Multiplexing:** Supports multiple concurrent client request.

## Usage

To run the web server, use the following command:

```bash
./webserv <configuration-file>
