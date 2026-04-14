# webserver
42_WebServer
*This project has been created as part of the 42 curriculum by 'imeslaki' & 'aazzaoui' & 'oel-bann' .*

# webserver

## Description
This repository contains a **C++98, event-driven HTTP web server** (42 “webserv”-style). The goal is to build a small server capable of handling multiple clients concurrently, parsing an **nginx-like configuration**, and serving HTTP responses (static content and other behaviors depending on configuration).

### High-level architecture (startup flow)
1. **Initialize** logging, signals, and file-descriptor (FD) limits
2. **Load configuration** (default: `EngineX/EngineX.conf` if no file is provided)
3. **Configuration pipeline**
   - **Tokenizing** → splits the config file into tokens
   - **Parsing** → builds an AST (syntax tree)
   - **Validation** → checks configuration correctness
   - **Fill runtime config** → converts AST into runtime `Config`
4. **Start the event loop** via a **Multiplexer** (poll/FD-based loop) to manage many connections efficiently

### Repository layout (top level)
- `Core/` — program entry point and orchestration
- `EngineX/` — configuration format/default configuration file(s)
- `Configuration/` — configuration parsing/validation related code (project-specific)
- `Network/` — socket/networking primitives
- `HTTP/` / `HTTP_Methods/` — HTTP parsing and method handling
- `conf/` — example configuration(s)

## Instructions

### Requirements
A Linux or macOS environment with:
- `c++` compiler supporting **C++98**
- `make`
- Standard POSIX networking APIs available

### Compile
From the repository root:
```sh
make
```

This produces:
- `webserv.out`

### Run
Run with the default configuration (auto-creates minimal defaults if needed):
```sh
./webserv.out
```

Run with an explicit configuration file:
```sh
./webserv.out path/to/your.conf
```

### Logging / Debug flags
The server supports debug flags **before** the config filename:

Global debug:
```sh
./webserv.out -d EngineX/EngineX.conf
```

Debug for a specific class/module name:
```sh
./webserv.out -dParsing EngineX/EngineX.conf
```

Detailed debug:
```sh
./webserv.out -D EngineX/EngineX.conf
```

### Clean
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

## Resources

### Web server / HTTP references
- **RFC 9110 — HTTP Semantics**
- **RFC 9112 — HTTP/1.1**
- **nginx documentation** (useful to understand nginx-style configuration concepts)

### Linux / event-driven I/O
- Linux/POSIX man pages: **`poll(2)`**, **`select(2)`**, **`epoll(7)`**
- Linux/POSIX sockets: **`socket(2)`**, **`bind(2)`**, **`listen(2)`**, **`accept(2)`**, **`fcntl(2)`** (non-blocking I/O)

### How AI was used
- AI was used to **understand the project and explain it**
- AI was **not** used to implement or modify the server source code.