#pragma once
#include "../Headers.hpp"
#include <sys/epoll.h>
class AFd;

#define BUFF_SIZE 1024
#define TIMEOUT_SEC 20

class Multiplexer {
	int epollFd;
	void epoolInit();
	static Multiplexer* currentMultiplexer;
public:
	Multiplexer();
	bool AddAsEpollIn(AFd *fd);
	bool AddAsEpollOut(AFd *fd);
	bool AddAsEpoll(AFd *fd, int type);
	bool ChangeToEpollOut(AFd *fd);
	static Multiplexer* GetCurrentMultiplexer();
	void MainLoop();
};
