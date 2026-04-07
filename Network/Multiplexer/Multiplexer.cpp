#include "Multiplexer.hpp"
#include "NetIO.hpp"
#include "SessionManager.hpp"
#include "HTTPContext.hpp"
#include "Config.hpp"

Multiplexer *Multiplexer::currentMultiplexer;

Multiplexer *Multiplexer::GetCurrentMultiplexer()
{
	return currentMultiplexer;
}

Multiplexer::Multiplexer(set<AFd *> &fds): _fds(fds)
{
	currentMultiplexer = this;
	count = 0;
	DEBUG("Multiplexer") << "Multiplexer initialized, starting epoll setup.";
	eventList.resize(64);
}

Multiplexer::~Multiplexer()
{
	close(epollFd);
}

void Multiplexer::epollInit()
{
	epollFd = epoll_create1(EPOLL_CLOEXEC);
	if (epollFd == -1)
		Error::ThrowError("epoll_create");
	DDEBUG("Multiplexer") << "epoll_create1 succeeded, epollFd=" << epollFd;
	set<AFd *>::iterator it = _fds.begin();
	DDEBUG("Multiplexer") << "Registering " << _fds.size() << " initial fd(s) as EPOLLIN.";
	for (; it != _fds.end(); it++)
	{
		AddAsEpollIn(*it);
	}
	DEBUG("Multiplexer") << "epollInit complete, " << _fds.size() << " fd(s) registered.";
}

bool Multiplexer::_modifyEpoll(int operation, AFd *fd, int type)
{
	if (fd->MarkedToDelete)
		return false;
	
	epoll_event ev;

	ev.events = type;
	ev.data.ptr = (void *)fd;
	if (epoll_ctl(epollFd, operation, fd->GetFd(), &ev) == -1)
	{
		DDEBUG("Multiplexer") << "epoll_ctl " << operation << " failed for fd=" << fd->GetFd() << ", type=" << fd->GetType();
		return false;
	}
	return true;
}

bool Multiplexer::AddAsEpollIn(AFd *fd)
{
	bool res = _modifyEpoll(EPOLL_CTL_ADD, fd, EPOLLIN);
	if (res)
		count++;
	return res;
}

bool Multiplexer::AddAsEpollOut(AFd *fd)
{
	bool res = _modifyEpoll(EPOLL_CTL_ADD, fd, EPOLLOUT);
	if (res)
		count++;
	return res;	
}

bool Multiplexer::ChangeToEpollIn(AFd *fd)
{
	return _modifyEpoll(EPOLL_CTL_MOD, fd, EPOLLIN);
}

bool Multiplexer::ChangeToEpollOut(AFd *fd)
{
	return _modifyEpoll(EPOLL_CTL_MOD, fd, EPOLLOUT);
}

bool Multiplexer::ChangeToEpollInOut(AFd *fd)
{
	return  _modifyEpoll(EPOLL_CTL_MOD, fd, EPOLLIN | EPOLLOUT);
}

bool Multiplexer::ChangeToEpollOneShot(AFd *fd)
{
	return  _modifyEpoll(EPOLL_CTL_MOD, fd, EPOLLONESHOT);
}

bool Multiplexer::DeleteFromEpoll(AFd *fd)
{
	bool res = epoll_ctl(epollFd, EPOLL_CTL_DEL, fd->GetFd(), NULL) == 0;
	DEBUG("Multiplexer") << "DeleteFromEpoll fd=" << fd->GetFd() << ", count=" << count << ", EPOLL_CTL_DEL: " << res;
	if (res)
		count--;
	return res;
}

void Multiplexer::MainLoop()
{
	epollInit();
	size_t Count = 0;
	INFO() << "Server is ready, waiting for connections...";
	while (true)
	{
		int timeout = NetIO::GetSocketTimeout();
		DDEBUG("Multiplexer") 
			<< "Starting epoll_wait with timeout=" << timeout 
			<< " ms, current count=" << count;
		if (count > (int)eventList.size())
			eventList.resize(count * 2);
		int size = epoll_wait(epollFd, eventList.data(), eventList.size(), timeout);
		for (int i = 0; i < size; i++)
		{
			_handleEpollAfd(eventList[i]);
			_tryCleanup(eventList[i]);
		}
		_cleanupDeletedList();
		NetIO::ClearTimeout();
		if (Utility::SigInt)
		{
			INFO() << "SIGINT received. Shutting down server.";
			break;
		}
		Logging::Info() << "epoll_wait, Handle: " << size << " AFd, loop iteration complete " << Count++ << "\n";
	}
}

void Multiplexer::_tryCleanup(epoll_event &event)
{
	AFd *obj = (AFd *)(event.data.ptr);
	if (obj->MarkedToDelete)
		return;
	
	if (event.events & MyEpollEvent::eEPOLLERR) {
		DDEBUG("Multiplexer") << "ClearEventObj() for fd=" << obj->GetFd() << ", event=" << event.events;
		ScheduleForDeletion(obj);
	}
}

void Multiplexer::_handleEpollAfd(epoll_event &event)
{
	AFd *obj = (AFd *)(event.data.ptr);
	
	if (obj->MarkedToDelete)
		return;
	DDEBUG("Multiplexer")
		<< "handleEpollSocket: fd=" << obj->GetFd()
		<< ", type=" << obj->GetType()
		<< ", events=" << event.events
		<< ", cleanBody=" << obj->cleanBody;
	
	obj->SetEvents(event.events);
	if (obj->cleanBody && (event.events & (MyEpollEvent::eEPOLLIN | EPOLLRDHUP)))
		obj->cleanFd();
	else if (event.events & (MyEpollEvent::eEPOLLIN | MyEpollEvent::eEPOLLOUT) || obj->GetType() == "Pipe")
		obj->Handle();
}

void Multiplexer::_cleanupDeletedList()
{
	for (int i = 0; i < (int)_deletedList.size(); i++)
	{
		AFd *obj = _deletedList[i];
		DeleteFromEpoll(obj);
		_fds.erase(obj);
		delete obj;
	}
	_deletedList.clear();
}

void Multiplexer::ScheduleForDeletion(AFd *obj)
{
	if (obj->MarkedToDelete) 
		return;
    
    obj->MarkedToDelete = true;
    _deletedList.push_back(obj);
}
