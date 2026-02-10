#include "Pipe.hpp"

void Pipe::Handle()
{
	socket->SetStateByFd(fd);
}

Pipe::Pipe(int fd, SocketIO *socket): AFd(fd, "Pipe"), socket(socket)
{}