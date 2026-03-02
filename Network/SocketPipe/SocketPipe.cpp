#include "SocketPipe.hpp"

void SocketPipe::Handle()
{
	if (MarkedToDelete)
		return;
	socket->SetStateByFd(fd);
	
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();
	MulObj->ChangeToEpollOneShot(this);
}

SocketPipe::SocketPipe(int fd, SocketIO *socket): APipe(fd, "Pipe"), socket(socket)
{}