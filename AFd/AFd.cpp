#include "AFd.hpp"

AFd::AFd(int fd, string type)
{
	this->fd = fd;
	this->type = type;
	MarkedToFree = false;
	markedTime = 0;
}

AFd::operator int() const 
{
    return fd;
}

string AFd::GetType()
{
	return type;
}

int AFd::GetFd()
{
	return fd;
}

AFd::~AFd() {}