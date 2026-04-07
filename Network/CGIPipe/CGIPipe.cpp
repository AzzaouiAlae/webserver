#include "CGIPipe.hpp"

void CGIPipe::Handle()
{
	if (MarkedToDelete)
		return;
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();
	MulObj->ChangeToEpollOneShot(this);
	if (cgi)
		cgi->SetStateByFd(fd);
}

CGIPipe::CGIPipe(int fd, Cgi *cgi): APipe(fd, "Pipe"), cgi(cgi)
{
	DDEBUG("CGIPipe") << "CGIPipe constructor called for fd=" << fd << ", address: " << this;
	Singleton::GetFds().insert(this);
}

CGIPipe::~CGIPipe()
{
	if (cgi)
		cgi->PipeClosed(fd);
}

void CGIPipe::CgiClosed()
{
	cgi = NULL;
}