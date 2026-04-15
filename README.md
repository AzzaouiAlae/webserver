# webserver
42_WebServer  
*This project has been created as part of the 42 curriculum by **imeslaki** & **aazzaoui** & **oel-bann**.*

## Description
This repository contains a **C++98, event-driven HTTP web server** (42 “webserv”-style).  
The goal is to build a small server capable of handling multiple clients concurrently, parsing an **nginx-like configuration**, and serving HTTP responses (static content + configured behaviors).

---

## High-level architecture (startup flow)
1. Initialize logging, signals, and file-descriptor limits  
2. Load configuration (default: `EngineX/EngineX.conf` if no file is provided)
3. Config pipeline:
   - Tokenizing → split config into tokens
   - Parsing → build an AST (syntax tree)
   - Validation → check config correctness
   - Fill runtime config → convert AST into runtime `Config`
4. Start the event loop (poll/FD-based multiplexer) to handle many clients efficiently

---

## Repository layout (top level)
- `Core/` — program entry point and orchestration
- `EngineX/` — configuration format + default config/www
- `Configuration/` — config parsing + validation code
- `Network/` — sockets/networking primitives
- `HTTP/` / `HTTP_Methods/` — HTTP parsing and method handling
- `conf/` — example configuration(s)

---

## Requirements
Linux or macOS with:
- `c++` compiler supporting **C++98**
- `make`
- POSIX networking APIs

---

## Build
From the repository root:
```sh
make
```

Output:
- `webserv.out`

---

## Run

Run with the default configuration (created automatically if missing):
```sh
./webserv.out
```

Run with an explicit configuration file:
```sh
./webserv.out path/to/your.conf
```

---

## Logging / Debug flags
Debug flags must be **before** the config filename.

Global debug:
```sh
./webserv.out -d EngineX/EngineX.conf
```

Debug for a specific module name:
```sh
./webserv.out -dParsing EngineX/EngineX.conf
```

Detailed debug:
```sh
./webserv.out -D EngineX/EngineX.conf
```

---

## Makefile helpers
Clean objects:
```sh
make clean
```

Remove binary too:
```sh
make fclean
```

Rebuild:
```sh
make re
```

---

## Configuration (nginx-like)

### File structure
A configuration file is mainly:
- one or more `server { ... }` blocks
- optional `types { ... }` block (MIME types)

Minimal example:
```conf
server {
}

types {
    text/html htm html;
}
```

### Supported directives (student-style quick table)

**Top-level**
- `server { ... }` — define one virtual server
- `types { ... }` — map MIME types to extensions

**Inside `server { ... }`**
| Directive | Example | Meaning |
|---|---|---|
| `listen` | `listen :8080;` / `listen 127.0.0.1:8080;` | bind address/port |
| `server_name` | `server_name localhost;` | virtual host name |
| `root` | `root EngineX/www;` | default web root |
| `index` | `index index.htm index.html;` | default index files |
| `autoindex` | `autoindex on;` | directory listing on/off |
| `client_max_body_size` | `client_max_body_size 1000000;` | max request body size |
| `allow_methods` | `allow_methods GET POST DELETE;` | allowed HTTP methods |
| `error_page` | `error_page 404 /HTTP/DefaultPages/Pages/404.html;` | custom error page mapping |
| `return` | `return 301 /new-path;` | redirect/return code |
| `keep_alive_timeout` | `keep_alive_timeout 60;` | keep-alive timeout |
| `client_read_timeout` | `client_read_timeout 30;` | read timeout |
| `cgi_timeout` | `cgi_timeout 10;` | CGI timeout |
| `location` | `location /images/ { ... }` | per-path rules |

**Inside `location <path> { ... }`**
| Directive | Example | Meaning |
|---|---|---|
| `root` | `root EngineX/www;` | override root for this location |
| `index` | `index index.htm;` | override index files |
| `autoindex` | `autoindex off;` | listing on/off |
| `client_max_body_size` | `client_max_body_size 2000000;` | override max body size |
| `allow_methods` | `allow_methods GET;` | override allowed methods |
| `return` | `return 302 /;` | redirect/return |
| `cgi_pass` *(if used)* | `cgi_pass .py /usr/bin/python3;` | run scripts via CGI |
| `method_redirect` *(project-specific)* | `method_redirect POST /upload/;` | method-based internal routing |

### `types { ... }` (MIME types)
Maps MIME type to extension(s):
```conf
types {
    text/html htm html;
    text/css css;
    application/javascript js;
}
```

### Example configuration
```conf
server {
    listen :8080;
    server_name localhost;

    root EngineX/www;
    index index.htm;

    autoindex off;
    client_max_body_size 1000000;
    allow_methods GET POST DELETE;

    error_page 404 /HTTP/DefaultPages/Pages/404.html;

    location / {
        root EngineX/www;
        index index.htm;
    }

    # Example CGI-style location (if enabled/configured)
    # location /cgi/ {
    #     cgi_pass .py /usr/bin/python3;
    # }
}

types {
    text/html htm html;
    text/css css;
    application/javascript js;
}
```

---

## Resources
- RFC 9110 — HTTP Semantics
- RFC 9112 — HTTP/1.1
- nginx docs (for config concepts)
- man pages: `poll(2)`, `select(2)`, `epoll(7)`, `socket(2)`, `bind(2)`, `listen(2)`, `accept(2)`, `fcntl(2)`

---

## How AI was used
- AI was used to **understand the project and explain it**
- AI was **not** used to implement or modify the server source code