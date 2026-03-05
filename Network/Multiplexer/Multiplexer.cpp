#include "Multiplexer.hpp"
#include "SocketIO.hpp"
#include "SessionManager.hpp"
#include "HTTPContext.hpp"

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
	DEBUG("Multiplexer") << "Multiplexer initialized, starting epoll setup.";
	epoolInit();
}

void Multiplexer::epoolInit()
{
	epollFd = epoll_create1(EPOLL_CLOEXEC);
	if (epollFd == -1)
		Error::ThrowError("epoll_create");
	DDEBUG("Multiplexer") << "epoll_create1 succeeded, epollFd=" << epollFd;
	set<AFd *> &fds = Singleton::GetFds();
	set<AFd *>::iterator it = fds.begin();
	DDEBUG("Multiplexer") << "Registering " << fds.size() << " initial fd(s) as EPOLLIN.";
	for(; it != fds.end(); it++)
	{
		AddAsEpollIn(*it);
	}
	DEBUG("Multiplexer") << "epoolInit complete, " << fds.size() << " fd(s) registered.";
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
	//INFO() << "AddAsEpoll addr: " << (void *)fd << ", fd: " <<  fd->GetFd() << ", type: " << fd->GetType();
	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd->GetFd(), &ev) == -1)
	{
		DDEBUG("Multiplexer") << "epoll_ctl ADD failed for fd=" << fd->GetFd() << ", type=" << fd->GetType();
		return false;
	}
	count++;
	DDEBUG("Multiplexer") << "epoll_ctl ADD succeeded for fd=" << fd->GetFd() << ", type=" << fd->GetType() << ", count=" << count;
	return true;
}

bool Multiplexer::ChangeToEpoll(AFd *fd, int type)
{
	epoll_event ev;

	ev.events = type;
	ev.data.ptr = (void *)fd;
	//INFO() << "ChangeToEpoll addr: " << (void *)fd << ", fd: " <<  fd->GetFd() << ", type: " << fd->GetType();
	if (epoll_ctl(epollFd, EPOLL_CTL_MOD, fd->GetFd(), &ev) == -1)
	{
		DDEBUG("Multiplexer") << "ChangeToEpoll: "<< type <<", failed for fd=" << fd->GetFd() << ", type=" << fd->GetType();
		return false;
	}
	DDEBUG("Multiplexer") << "ChangeToEpoll: "<< type <<", succeeded for fd=" << fd->GetFd() << ", type=" << fd->GetType();
	return true;
}

bool Multiplexer::ChangeToEpollOut(AFd *fd)
{
	return ChangeToEpoll(fd, EPOLLOUT);
}

bool Multiplexer::ChangeToEpollIn(AFd *fd)
{
	return ChangeToEpoll(fd, EPOLLIN);
}

bool Multiplexer::ChangeToEpollOneShot(AFd *fd)
{
	return ChangeToEpoll(fd, EPOLLONESHOT);
}

bool Multiplexer::DeleteFromEpoll(AFd *fd)
{
	count--;
	bool res = epoll_ctl(epollFd, EPOLL_CTL_DEL, fd->GetFd(), NULL) == 0;
	DDEBUG("Multiplexer") << "DeleteFromEpoll fd=" << fd->GetFd() << ", count=" << count << ", EPOLL_CTL_DEL: " << res;
	//INFO() << "DeleteFromEpoll addr: " << (void *)fd << ", fd: " <<  fd->GetFd() << ", type: " << fd->GetType();
	return res;
}

bool Multiplexer::ChangeToEpollInOut(AFd *fd)
{
	return ChangeToEpoll(fd, EPOLLIN | EPOLLOUT);
}

void Multiplexer::MainLoop()
{
	size_t Count = 0;
	INFO() << "Server is ready, waiting for connections...";
	while(true)
	{
		epoll_event eventList[count];
		int size = epoll_wait(epollFd, eventList, count, USEC * TIMEOUT / 1000);

		for(int i = 0; i < size; i++) {
			handelEpollPipes(eventList[i]);
		}
		for(int i = 0; i < size; i++) {
			handelEpollSocket(eventList[i]);
		}
		ClearToDelete();
		if (Utility::SigInt) {
			INFO() << "SIGINT received. Shutting down server.";
			break;
		}
		INFO() << "epoll_wait loop iteration complete " << Count++ << "\n";
	}
}

void Multiplexer::DeleteItem(AFd *item)
{
	tcp_info info;
    socklen_t len;
	len = sizeof(info);

	if (item->deleteNow || item->GetType() == "Socket")
	{
		delete item;
        toDelete.erase(item);
	}
	else if (getsockopt(item->GetFd(), IPPROTO_TCP, TCP_INFO, &info, &len) == 0)
	{
		if (info.tcpi_unacked == 0)
			item->deleteNow = true;
	}
}

void Multiplexer::ClearToDelete()
{
	SocketIO::CloseSockFD(-1);
	SocketIO::clearTimeout();
	
	if (toDelete.size() == 0)
		return;
	DDEBUG("Multiplexer") << "ClearToDelete: " << toDelete.size() << " item(s) pending deletion.";
	set<AFd *>::iterator it = toDelete.begin();
	set<AFd *>::iterator next;
	SessionManager *p = SessionManager::getInstance();
	if (p != NULL)
	{
		p->cleanupExpiredSessions();
	}
	for(; it != toDelete.end(); it = next)
	{
		next = it;
		next++;
		DeleteItem(*it);
	}
}

Multiplexer::~Multiplexer()
{
	DEBUG("Multiplexer") << "Multiplexer destructor called, cleaning up.";
	set<AFd *> &fds = Singleton::GetFds();
	set<AFd *>::iterator it = fds.begin();

	for(; it != fds.end(); it++)
	{
		DEBUG("Multiplexer") << "Deleting fd=" << (*it)->GetFd() << ", type=" << (*it)->GetType();
		toDelete.insert(*it);
	}
	DDEBUG("Multiplexer") << "Marked " << fds.size() << " fd(s) for deletion, closing epollFd=" << epollFd;
	close(epollFd);
	while (toDelete.size()) {
		DEBUG("Multiplexer") << "Waiting for " << toDelete.size() << " item(s) to be deleted.";
		ClearToDelete();
	}
	while (SocketIO::CloseSockFD(-1)) {
		DEBUG("Multiplexer") << "Waiting for all socket fds to be closed.";
	}
	APipe::ClearPipePool();
	SessionManager *p = SessionManager::getInstance();
	delete p;
	delete[] AFd::buff;
	HTTPContext::ClearBuffPoll();
	DEBUG("Multiplexer") << "Multiplexer cleanup complete.";
}

bool Multiplexer::ClearEventObj(epoll_event &event)
{
	AFd *obj = (AFd *)(event.data.ptr);

	if (obj->MarkedToDelete || (event.events & (EPOLLIN | EPOLLOUT)) == 0)
	{
		ClearObj(obj);
		return true;
	}
	return false;
}

void Multiplexer::ClearObj(AFd *obj)
{
	DEBUG("Multiplexer") << "Socket fd: " << obj->GetFd() << ", MarkedToDelete";
	obj->MarkedToDelete = true;
	if (currentMultiplexer == NULL)
		return;
	currentMultiplexer->DeleteFromEpoll(obj);
	toDelete.insert(obj);
}

void Multiplexer::handelEpollPipes(epoll_event &event)
{
	AFd *obj = (AFd *)(event.data.ptr);

	if (obj->GetType() == "Pipe") 
	{
		DDEBUG("Multiplexer") << "handelEpollPipes: handling Pipe fd=" << obj->GetFd();
		obj->Handle();
	}
}

void Multiplexer::handelEpollSocket(epoll_event &event)
{
	AFd *obj = (AFd *)(event.data.ptr);
	
	if (obj->GetType() == "Pipe") 
		return;
	DDEBUG("Multiplexer") 
		<< "handelEpollSocket: fd=" << obj->GetFd()
		<< ", type=" << obj->GetType()
		<< ", events=" << event.events
		<< ", cleanBody=" << obj->cleanBody;
	if (obj->cleanBody && (event.events & EPOLLIN)) {
		DDEBUG("Multiplexer") << "  -> cleanFd() for fd=" << obj->GetFd();
		obj->cleanFd();
	}
	else if (ClearEventObj(event))
		return;
	else if (event.events & (EPOLLIN | EPOLLOUT)) {
		DDEBUG("Multiplexer") << "  -> Handle() for fd=" << obj->GetFd();
		obj->Handle();
	}
}
