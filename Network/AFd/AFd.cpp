#include "AFd.hpp"
#include "Headers.hpp"

#define CLEAN_SIZE 1024 * 1024 * 1

char *AFd::buff;

AFd::AFd(int fd, string type)
{
	this->fd = fd;
	this->type = type;
	MarkedToDelete = false;
	cleanBody = false;
	totalClean = 0;
	DDEBUG("AFd") << "AFd constructor called for fd=" << fd << ", type=" << type << ", address: " << this;
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
{
	
	close(fd);
	DDEBUG("AFd") << "AFd destructor called for fd=" << fd << ", type=" << type << ", address: " << this;
}

void AFd::cleanFd()
{
	if (buff == NULL)
		buff = Utility::GetBuffer();
	int size = read(fd, buff, BUF_SIZE);
	if (size > 0)
		totalClean += size;
	if (totalClean  >= CLEAN_SIZE || size <= 0 || Utility::SigPipe) 
	{
		Utility::SigPipe = false;
		cleanBody = false;
		Multiplexer::GetCurrentMultiplexer()->ScheduleForDeletion(this);
	}
	DDEBUG("AFd") << "sock fd: " << fd << ", AFd::cleanFd() read " << size;
}

void AFd::SetEvents(unsigned int ev) { events = ev; }

unsigned int AFd::GetEvents() const { return events; }