#pragma once
#include "../Headers.hpp"
#include "../SocketIO/SocketIO.hpp"

class Pipe : public AFd {
	SocketIO *socket;
public:
	Pipe(int fd, SocketIO *socket);
	void Handle();
};
