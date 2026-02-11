#include "Multiplexer.hpp"

Multiplexer* Multiplexer::currentMultiplexer;

Multiplexer* Multiplexer::GetCurrentMultiplexer()
{
	return currentMultiplexer;
}

Multiplexer::Multiplexer()
{
	currentMultiplexer = this;
	epoolInit();
}

void Multiplexer::epoolInit()
{
	epollFd = epoll_create1(EPOLL_CLOEXEC);
	if (epollFd == -1)
		Error::ThrowError("epoll_create");
	vector<AFd *> &fds = Singleton::GetFds();
	
	for(int i = 0; i < (int)fds.size(); i++)
	{
		AddAsEpollIn(fds[i]);
	}
}

bool Multiplexer::AddAsEpollIn(AFd *fd)
{
	return AddAsEpoll(fd, EPOLLIN);
}

bool Multiplexer::AddAsEpollOut(AFd *fd)
{
	return AddAsEpoll(fd, EPOLLOUT);
}

bool Multiplexer::AddAsEpoll(AFd *fd, int type)
{
	epoll_event ev;

	ev.events = type;
	ev.data.ptr = (void *)fd;

	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd->GetFd(), &ev) == -1)
		return false;
	return true;
}

bool Multiplexer::ChangeToEpollOut(AFd *fd)
{
	epoll_event ev;

	ev.events = EPOLLOUT;
	ev.data.ptr = (void *)fd;

	if (epoll_ctl(epollFd, EPOLL_CTL_MOD, fd->GetFd(), &ev) == -1)
		return false;
	return true;
}

bool Multiplexer::DeleteFromEpoll(AFd *fd)
{
	return epoll_ctl(epollFd, EPOLL_CTL_DEL, fd->GetFd(), NULL);
}

void Multiplexer::MainLoop()
{
	long time = Utility::CurrentTime() + USEC * 30;
	while(time > Utility::CurrentTime())
	{
		int s = Singleton::GetFds().size();
		epoll_event eventList[s];
		int size = epoll_wait(epollFd, eventList, s, -1);
		for(int i = 0; i < size; i++)
		{
			AFd *obj = (AFd *)(eventList[i].data.ptr);
			obj->Handle();
			if (obj->MarkedToFree)
				delete obj;
		}
	}
}
Multiplexer::~Multiplexer()
{
	vector<AFd *> &fds = Singleton::GetFds();
	
	for(int i = 0; i < (int)fds.size(); i++)
	{
		delete fds[i];
	}
}