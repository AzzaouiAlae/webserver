#pragma once
#include "Headers.hpp"
#include <sys/epoll.h>

class AFd;

class MyEpollEvent
{
	public:
	enum EpollEvents
	{
		eEPOLLIN = (EPOLLRDBAND | EPOLLRDNORM | EPOLLPRI | EPOLLIN),
		eEPOLLOUT = (EPOLLOUT | EPOLLWRNORM | EPOLLWRBAND),
		eEPOLLERR = (EPOLLERR | EPOLLHUP | EPOLLRDHUP)
	};
};

class Multiplexer
{
	vector<AFd *> _deletedList;
	int epollFd;
	void epollInit();
	static Multiplexer *currentMultiplexer;
	int count;
	vector<epoll_event> eventList;
	set<AFd *> &_fds;
	bool _modifyEpoll(int operation, AFd *fd, int type);
	void _tryCleanup(epoll_event &event);
	void _handleEpollAfd(epoll_event &event);
	void _cleanupDeletedList();
	
public:
	Multiplexer(set<AFd *> &fds);
	~Multiplexer();
	bool AddAsEpollIn(AFd *fd);
	bool AddAsEpollOut(AFd *fd);
	bool ChangeToEpollIn(AFd *fd);
	bool ChangeToEpollOut(AFd *fd);
	bool ChangeToEpollInOut(AFd *fd);
	bool StopEpollEvents(AFd *fd);
	bool DeleteFromEpoll(AFd *fd);
	static Multiplexer *GetCurrentMultiplexer();
	void MainLoop();
	void ScheduleForDeletion(AFd *obj);
};
