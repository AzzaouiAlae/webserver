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
	set<AFd *> &fds = Singleton::GetFds();
	set<AFd *>::iterator it = fds.begin();
	for(; it != fds.end(); it++)
	{
		AddAsEpollIn(*it);
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

bool Multiplexer::ChangeToEpollIn(AFd *fd)
{
	epoll_event ev;

	ev.events = EPOLLIN;
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

void Multiplexer::ChangeToEpollInOut(AFd *fd)
{
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLOUT;
    ev.data.fd = fd->GetFd();
    epoll_ctl(epollFd, EPOLL_CTL_MOD, fd->GetFd(), &ev);
}

void Multiplexer::MainLoop()
{
	int timeout = 1;
	long time = Utility::CurrentTime() + USEC * timeout;
	(void)time;
	// while(time > Utility::CurrentTime())
	while(true)
	{
		epoll_event eventList[count];
		int size = epoll_wait(epollFd, eventList, count, USEC * timeout / 1000);
		
		
		for(int i = 0; i < size; i++) {
			handelEpollPipes(eventList[i]);
		}
		for(int i = 0; i < size; i++) {
			handelEpollEvent(eventList[i]);
		}
		ClearToDelete();
	}
}

bool Multiplexer::DeleteItem(AFd *item)
{
	tcp_info info;
    socklen_t len;
	
	if (item->deleteNow)
	{
		delete item;
        toDelete.erase(item);
		return true;
	}
	len = sizeof(info);
	if (getsockopt(item->GetFd(), IPPROTO_TCP, TCP_INFO, &info, &len) == 0)
	{
		if (info.tcpi_unacked == 0 && item->deleteNow == 0)
		{
			item->deleteNow = true;
		}
	}
	return false;
}

void Multiplexer::ClearToDelete()
{
	SocketIO::CloseSockFD(-1);
	
	if (toDelete.size() == 0)
		return;
	set<AFd *>::iterator it = toDelete.begin();
	set<AFd *>::iterator next;
	for(; it != toDelete.end(); it = next)
	{
		next = it;
		next++;
		DeleteItem(*it);
	}
}

Multiplexer::~Multiplexer()
{
	set<AFd *> &fds = Singleton::GetFds();
	set<AFd *>::iterator it = fds.begin();

	for(; it != fds.end(); it++)
	{
		toDelete.insert(*it);
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

bool Multiplexer::ClearObj(epoll_event &event)
{
	AFd *obj = (AFd *)(event.data.ptr);

	if (obj->MarkedToDelete || event.events & (EPOLLERR | EPOLLPRI | EPOLLRDHUP))
	{
		DEBUG() << "Socket fd: " << obj->GetFd() << ", MarkedToDelete";
		obj->MarkedToDelete = true;
		DeleteFromEpoll(obj);
		toDelete.insert(obj);
		return true;
	}
	return false;
}

void Multiplexer::handelEpollPipes(epoll_event &event)
{
	AFd *obj = (AFd *)(event.data.ptr);

	if (obj->GetType() == "Pipe") 
		obj->Handle();
}

void Multiplexer::handelEpollEvent(epoll_event &event)
{
	AFd *obj = (AFd *)(event.data.ptr);
	
	if (obj->GetType() == "Pipe") 
		return;
	if (obj->cleanBody) {
		obj->cleanFd();
	}
	else if (ClearObj(event) == false && event.events & (EPOLLIN | EPOLLOUT)) {
		obj->Handle();
	}
}

