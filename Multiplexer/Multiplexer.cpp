#include "Multiplexer.hpp"
#include "../SocketIO/SocketIO.hpp"

set<AFd *> Multiplexer::toDelete;

Multiplexer* Multiplexer::currentMultiplexer;

Multiplexer* Multiplexer::GetCurrentMultiplexer()
{
	return currentMultiplexer;
}

Multiplexer::Multiplexer()
{
	currentMultiplexer = this;
	count = 0;
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
	count++;
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

bool Multiplexer::ChangeToEpollOneShot(AFd *fd)
{
	epoll_event ev;

	ev.events = EPOLLONESHOT;
	ev.data.ptr = (void *)fd;

	if (epoll_ctl(epollFd, EPOLL_CTL_MOD, fd->GetFd(), &ev) == -1)
		return false;
	return true;
}

bool Multiplexer::DeleteFromEpoll(AFd *fd)
{
	count--;
	return epoll_ctl(epollFd, EPOLL_CTL_DEL, fd->GetFd(), NULL);
}

void Multiplexer::MainLoop()
{
	int timeout = 15;
	AFd *obj;
	long time = Utility::CurrentTime() + USEC * timeout;
	(void)time;
	while(true)
	{
		epoll_event eventList[count];
		int size = epoll_wait(epollFd, eventList, count, USEC * timeout / 1000);
		for(int i = 0; i < size; i++)
		{
			obj = (AFd *)(eventList[i].data.ptr);
			if (obj->GetType() == "Pipe") {
				obj->Handle();
			}
		}
		for(int i = 0; i < size; i++)
		{
			obj = (AFd *)(eventList[i].data.ptr);
			if (obj->MarkedToFree == false && eventList->events & (EPOLLIN | EPOLLOUT)) {
				obj->Handle();
			}

			if (obj->MarkedToFree || eventList->events & (EPOLLERR | EPOLLPRI | EPOLLRDHUP))
			{
				obj->MarkedToFree = true;
				ChangeToEpollOneShot(obj);
				toDelete.insert(obj);
			}
		}
		ClearToDelete();
	}
}

bool Multiplexer::DeleteItem(AFd *item)
{
	long now = Utility::CurrentTime();
	tcp_info info;
    socklen_t len;
	
	if (item->markedTime && item->markedTime < now)
	{
		delete item;
        toDelete.erase(item);
		return true;
	}
	if (item->markedTime)
		return false;
	len = sizeof(info);
	if (getsockopt(item->GetFd(), IPPROTO_TCP, TCP_INFO, &info, &len) == 0)
	{
		if (info.tcpi_unacked == 0 && item->markedTime == 0)
		{
			item->markedTime = Utility::CurrentTime() + USEC;
		}
	}
	return false;
}

void Multiplexer::ClearToDelete()
{
	if (toDelete.size() == 0)
		return;
	set<AFd *>::iterator it = toDelete.begin();
	for(; it != toDelete.end(); it++)
	{
		if (DeleteItem(*it))
			break;
	}
}

Multiplexer::~Multiplexer()
{
	vector<AFd *> &fds = Singleton::GetFds();
	
	for(int i = 0; i < (int)fds.size(); i++)
	{
		delete fds[i];
	}
	close(epollFd);
	while (toDelete.size()) {
		ClearToDelete();
		usleep(10000);
	}
	while (SocketIO::CloseSockFD(-1)) {
		usleep(10000);
	}
	SocketIO::ClearPipePool();
}
