#pragma once
#include "../Headers.hpp"
#include <sys/epoll.h>

#define BUFF_SIZE 1024
#define TIMEOUT_SEC 20

class Multiplexer {

public:
	void MainLoop();
};
