#include "ISocket.hpp"

ISocket::ISocket(int fd, string type): AFd(fd, type)
{}