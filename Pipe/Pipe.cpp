#include "Pipe.hpp"

void Pipe::Handle()
{
	if (MarkedToFree)
		return;
	socket->SetStateByFd(fd);
}

Pipe::Pipe(int fd, SocketIO *socket): AFd(fd, "Pipe"), socket(socket)
{}