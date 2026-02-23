#include "AFd.hpp"
#include "../../Headers.hpp"

char *AFd::buff;

AFd::AFd(int fd, string type)
{
	this->fd = fd;
	this->type = type;
	MarkedToDelete = false;
	deleteNow = false;
	cleanBody = false;
	totalClean = 0;
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

AFd::~AFd() 
{}

void AFd::cleanFd()
{
	if (buff == NULL)
		buff = (char *)calloc(1, 1024 * 1024 * 5);
	int size = 0;
	size = read(fd, buff, 1024 * 1024 * 5);
	if (size > 0)
		totalClean += size;
	if (totalClean  >= maxToClean || size == 0 || Utility::SigPipe) 
	{
		Utility::SigPipe = false;
		cleanBody = false;
		MarkedToDelete = true;
	}
	DEBUG("AFd") << "sock fd: " << fd << ", AFd::cleanFd() read " << size;
}
