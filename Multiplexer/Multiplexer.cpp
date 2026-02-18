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



void Multiplexer::MainLoop()
{
	int timeout = 10;
	AFd *obj;
	long time = Utility::CurrentTime() + USEC * timeout;
	(void)time;
	// while(time > Utility::CurrentTime())
	while(true)
	{
		epoll_event eventList[count];
		int size = epoll_wait(epollFd, eventList, count, USEC * timeout / 1000);
		Logging::Debug() << "New event/timeout happen size: " << size;
		
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
			if (obj->GetType() == "Pipe") {
				continue;
			}
			if (obj->MarkedToDelete == false && eventList->events & (EPOLLIN | EPOLLOUT)) {
				Logging::Debug() << "Start handel " << obj->GetType() << 
					" with fd: " << obj->GetFd() << " and flag " <<
					((eventList->events & EPOLLIN) ? "EPOLLIN" : "EPOLLOUT");
				obj->Handle();
			}

			if (obj->MarkedToDelete || eventList->events & (EPOLLERR | EPOLLPRI | EPOLLRDHUP))
			{
				Logging::Debug() << "Add " << obj->GetType() << 
					" with fd: " << obj->GetFd() << " to delete";
				obj->MarkedToDelete = true;
				DeleteFromEpoll(obj);
				toDelete.insert(obj);
			}
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
		Logging::Debug() << item->GetType() << 
					" with fd: " << item->GetFd() << " deleted";
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
	Logging::Debug() << "toDelete has " << toDelete.size()
	<< ", SocketIO has " << SocketIO::CloseSockFD(-1) <<
	", pipe poll has " << SocketIO::GetPipePoolSize();
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
