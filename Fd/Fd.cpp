#include "../Routing/Routing.hpp"
#include "Fd.hpp"

Fd::Fd(int fd, Fd::Types type, Fd::Status status)
{
	this->fd = fd;
	this->type = type;
	this->status = status;
	time = Utility::CurrentTime();
}

Fd::operator int() const 
{
    return fd;
}

bool Fd::isTimeOut()
{
	long now = Utility::CurrentTime();
	if (now - time >= TIMEOUT_SEC * USEC)
		return true;
	return false;
}