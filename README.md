# webserver
42_WebServer  
*This project has been created as part of the 42 curriculum by **imeslaki** & **aazzaoui** & **oel-bann**.*

## Description
A **C++98, event-driven HTTP web server** (42 “webserv”-style).  
It handles multiple clients concurrently using **epoll**, parses an **nginx-like configuration**, and serves HTTP responses (static content + configured behaviors like CGI, uploads, etc.).

---

## Features (high level)
- C++98 / POSIX networking
- Event-driven concurrency (single/few threads, many sockets) via **epoll**
- nginx-like configuration: `server {}` / `location {}` / `types {}`
- HTTP parsing + routing + method dispatch (GET/POST/DELETE)
- Strategy-based body reading/sending (multipart, chunked, buffered, file)
- CGI support (pipes + request/response handling)
- Session management module (project-specific)

---

## Repository layout (top level)
- `Core/` — program entry point and orchestration
- `EngineX/` — configuration format + default config/www
- `Configuration/` — config tokenizing/parsing/validation
- `Network/` — sockets + multiplexer (epoll)
- `HTTP/` / `HTTP_Methods/` — HTTP parsing, routing, method handlers, strategies
- `conf/` — example configuration(s)

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

Default configuration (auto-created if missing):
```sh
./webserv.out
```

Run with explicit config file:
```sh
./webserv.out path/to/your.conf
```

---

## Logging / Debug flags
> Debug flags must be **before** the config filename.

Global debug:
```sh
./webserv.out -d EngineX/EngineX.conf
```

Debug for a specific module:
```sh
./webserv.out -dParsing EngineX/EngineX.conf
```

Detailed debug:
```sh
./webserv.out -D EngineX/EngineX.conf
```

---

## Configuration (nginx-like)

### Minimal example
```conf
server {
}

types {
    text/html htm html;
}
```

### Supported directives (quick table)

**Top-level**
- `server { ... }`
- `types { ... }`

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
| `root` | `root EngineX/www;` | override root |
| `index` | `index index.htm;` | override index |
| `autoindex` | `autoindex off;` | listing |
| `client_max_body_size` | `client_max_body_size 2000000;` | override max body |
| `allow_methods` | `allow_methods GET;` | override allowed methods |
| `return` | `return 302 /;` | redirect/return |
| `cgi_pass` | `cgi_pass .py /usr/bin/python3;` | CGI mapping |
| `method_redirect` | `method_redirect POST /upload/;` | method-based routing |

---

# Architecture (current version)

This section follows the schema in `HTTP/DefaultPages/server2.svg` and matches the runtime behavior.

## 1) Startup flow (from `main`)
At startup, the server prepares configuration and runtime state, then enters the event loop.

1. **Logging setup** (log file, debug flags)
2. **Signals + FD limits**
   - `SIGPIPE` ignored/handled
   - `SIGINT` triggers graceful shutdown
   - FD limits inspected to size internal structures
3. **Config pipeline**
   - `Tokenizing` → split config into tokens
   - `Parsing` → build an AST
   - `Validation` → validate AST rules
   - `Config::FillConf()` → build runtime `Config` used by networking & HTTP layers
4. **Default pages init**
   - `DefaultPages::InitDefaultPages()`
5. **Start event loop**
   - `Multiplexer::MainLoop()` (epoll)

> In code: `Core/main.cpp` creates `Multiplexer m(Singleton::GetFds());` and calls `m.MainLoop();`

---

## 2) Main loop (epoll-based Multiplexer)

### What it is
The **Multiplexer** is the central event loop.  
It registers all active file descriptors (listening sockets, client sockets, pipes) and repeatedly waits for I/O readiness using `epoll_wait`.

### Main loop flow (matches `Network/Multiplexer/Multiplexer.cpp`)
1. **Initialize epoll**
   - `epoll_create1(EPOLL_CLOEXEC)`
   - register initial fds (`AddAsEpollIn`)

2. **Repeat**
   - compute timeout (`NetIO::GetSocketTimeout()`)
   - `epoll_wait(...)`
   - for each event:
     - `_handleEpollAfd(event)`
       - set events on the AFd (`obj->SetEvents`)
       - if `cleanBody` and input/hup → `obj->cleanFd()`
       - else dispatch to polymorphic handler: `obj->Handle()`
     - `_tryCleanup(event)`
       - if error (`EPOLLERR | EPOLLHUP | EPOLLRDHUP`) schedule deletion
   - delete scheduled objects (`_cleanupDeletedList`)
   - clear timeout state (`NetIO::ClearTimeout()`)

3. **Graceful shutdown**
   - if `Utility::SigInt` is set → break loop

### Why this design
- **Scales**: many clients without a thread per client
- **Extensible**: all I/O objects are modeled as `AFd` (socket, pipe, etc.)
- **Safe cleanup**: objects are deleted after the iteration (deferred deletion list)

---

## 3) Core runtime object model (schema mapping)

### AFd (base abstraction)
`AFd` represents “anything that has a file descriptor and reacts to epoll events”.

Examples of AFd-like entities in the diagram:
- `ISocket` and its implementations (`Socket`, `ClientSocket`)
- `APipe` and special pipes (`CGIPipe`)
- Other FD-based components

**Multiplexer owns the loop**, but **AFd owns the behavior** (`Handle()`).

---

## 4) Connection and request pipeline

This is the lifecycle from “socket is readable” to “response is written”.

### 4.1 ConnectionContext / HTTPContext
- A new connection is represented by a `ConnectionContext`.
- HTTP-specific state is represented by an `HTTPContext`.
- The context holds current parsing state, request buffer, routing results, etc.

### 4.2 ClientRequest
Incoming bytes are parsed into a `ClientRequest`:
- method (GET/POST/DELETE)
- path
- headers
- body metadata (content-length / chunked / multipart)

### 4.3 Routing
`Routing` decides what should happen based on:
- server block match (listen/server_name)
- location match
- allowed methods
- root/index/autoindex
- return / redirects
- CGI mapping / upload paths / error pages

Routing outputs an execution plan for the request: static file, directory listing, CGI, upload strategy, etc.

---

## 5) Method dispatch (AMethod → GET/POST/DELETE)
After routing:
- the server selects an `AMethod` implementation depending on the HTTP method:
  - `GET`
  - `POST`
  - `DELETE`

Each method handler:
- validates method rules
- triggers appropriate read strategy (for request body)
- triggers appropriate send strategy (for response body)

---

## 6) Strategies (body reading and response sending)

The diagram includes a strategy layer that abstracts *how* body data is read/written.

### Read strategies (request body)
- `ChunkedData` (Transfer-Encoding: chunked)
- `MultipartData` (multipart/form-data uploads)
- Other buffered reads

### Send strategies (response body)
- `FileStrategy` (serve static files)
- `BuffersStrategy` (serve from memory buffers)
- Upload / multipart strategies shown in schema:
  - `UploadStrategy`
  - `MultipartUploadStrategy`

This design keeps HTTP method logic clean while allowing multiple body encodings.

---

## 7) CGI execution path (pipes + CGIRequest)

When a route is configured for CGI:
1. Request becomes a `CGIRequest` (a specialized request execution)
2. A `CGIPipe` (pipe FD(s)) is created to communicate with the CGI process
3. Strategies manage pipe I/O:
   - `WriteToPipeStrategy` (send request body to CGI stdin)
   - `ReadPipeStrategy` / `ReadCGIStrategy` (read CGI stdout)
   - buffered/chunked writers (as shown in schema):
     - `WriteBufferedCGIStrategy`
     - `WriteChunkedCGIStrategy`
4. The CGI output is transformed into an HTTP response.

Because pipes are also file descriptors, they are handled by the same `Multiplexer` loop.

---

## 8) Session management (project-specific)
A `SessionManagement` module exists in the schema and integrates with request handling:
- reads/writes cookies/session identifiers
- may store session data (implementation-specific)

---

## Resources
- RFC 9110 — HTTP Semantics
- RFC 9112 — HTTP/1.1
- nginx docs (for config concepts)
- man pages: `epoll(7)`, `socket(2)`, `bind(2)`, `listen(2)`, `accept(2)`, `fcntl(2)`

---

## How AI was used
- AI was used to **understand the project and explain it**
- AI was **not** used to implement or modify the server source code