#pragma once
#include "../Headers.hpp"
#include <sys/epoll.h>

#define BUFF_SIZE 1024

class Multiplexer {

public:
	void MainLoop();
};
