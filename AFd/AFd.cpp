#include "AFd.hpp"
#include "../Headers.hpp"

AFd::AFd(int fd, string type)
{
	this->fd = fd;
	this->type = type;
	MarkedToDelete = false;
	deleteNow = false;
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