#pragma once
#include "../Headers.hpp"
#include <sys/epoll.h>
class AFd;

#define TIMEOUT_SEC 20

class Multiplexer {
	int epollFd;
	void epoolInit();
	static Multiplexer* currentMultiplexer;
	static set<AFd *> toDelete;
	int count;
	static bool DeleteItem(AFd *item);
public:
	void ChangeToEpollInOut(AFd *fd);
	bool ChangeToEpollOneShot(AFd *fd);
	Multiplexer();
	~Multiplexer();
	bool AddAsEpollIn(AFd *fd);
	bool AddAsEpollOut(AFd *fd);
	bool AddAsEpoll(AFd *fd, int type);
	bool ChangeToEpollOut(AFd *fd);
	bool DeleteFromEpoll(AFd *fd);
	bool ChangeToEpollIn(AFd *fd);
	static Multiplexer* GetCurrentMultiplexer();
	void MainLoop();
	void ClearToDelete();
	bool ClearObj(epoll_event &event);
	void handelEpollPipes(epoll_event &event);
	void handelEpollEvent(epoll_event &event);

};
