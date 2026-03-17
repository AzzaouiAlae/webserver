#pragma once
#include "Headers.hpp"
#include <sys/epoll.h>

class AFd;

#define MYEPOLLIN (EPOLLRDBAND | EPOLLRDNORM | EPOLLPRI | EPOLLIN)
#define MYEPOLLOUT (EPOLLOUT | EPOLLWRNORM | EPOLLWRBAND)
#define MYEPOLLERR (EPOLLERR | EPOLLHUP)

class Multiplexer
{
	int epollFd;
	void epoolInit();
	static Multiplexer *currentMultiplexer;
	static set<AFd *> toDelete;
	int count;
	static void DeleteItem(AFd *item);

public:
	Multiplexer();
	~Multiplexer();
	bool AddAsEpollIn(AFd *fd);
	bool AddAsEpollOut(AFd *fd);
	bool AddAsEpoll(AFd *fd, int type);
	bool ChangeToEpoll(AFd *fd, int type);
	bool ChangeToEpollIn(AFd *fd);
	bool ChangeToEpollOut(AFd *fd);
	bool ChangeToEpollInOut(AFd *fd);
	bool ChangeToEpollOneShot(AFd *fd);
	bool DeleteFromEpoll(AFd *fd);
	static Multiplexer *GetCurrentMultiplexer();
	void MainLoop();
	void ClearToDelete();
	bool ClearEventObj(epoll_event &event);
	static void ClearObj(AFd *obj);
	void handelEpollPipes(epoll_event &event);
	void handelEpollAfd(epoll_event &event);
	static void keepSocketAlive(AFd *item);
};
