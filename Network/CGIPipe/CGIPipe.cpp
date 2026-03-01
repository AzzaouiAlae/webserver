#include "CGIPipe.hpp"

void CGIPipe::Handle()
{
	if (MarkedToDelete)
		return;
	cgi->SetStateByFd(fd);
	
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();
	MulObj->ChangeToEpollOneShot(this);
}

CGIPipe::CGIPipe(int fd, Cgi *cgi): APipe(fd, "Pipe"), cgi(cgi)
{}