#include "Pipe.hpp"

void Pipe::Handle()
{
	if (MarkedToDelete)
		return;
	socket->SetStateByFd(fd);
	
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();
	MulObj->ChangeToEpollOneShot(this);
}

Pipe::Pipe(int fd, SocketIO *socket): AFd(fd, "Pipe"), socket(socket)
{}