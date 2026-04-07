#include "APipe.hpp"

APipe::APipe(int fd, string type): AFd(fd, type)
{}

bool APipe::GetPipe(int pipe[2])
{
	if (pipe2(pipe, O_NONBLOCK | O_CLOEXEC) == -1)
	{
		ERR() << "SocketIO created pipe failed!";
		return false;
	}
	DEBUG("SocketIO") << "SocketIO pipe created [" << pipe[0] << ", " << pipe[1] << "]";
	return true;
}
