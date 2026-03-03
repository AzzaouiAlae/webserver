#pragma once
#include "Headers.hpp"
#include "SocketIO.hpp"

class SocketPipe : public APipe {
	SocketIO *socket;
public:
	SocketPipe(int fd, SocketIO *socket);
	void Handle();
};
