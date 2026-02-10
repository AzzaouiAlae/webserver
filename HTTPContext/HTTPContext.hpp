#pragma once
#include "../Headers.hpp"
#include "../Socket/Socket.hpp"
#include "../SocketIO/SocketIO.hpp"
#include "../Pipe/Pipe.hpp"

class HTTPContext : public IContext
{
public:
	vector<AST<string> > *servers;
	void Handle(AFd *fd);
	void Handle(Socket *sock);
	void Handle(SocketIO *sock);
	void HandleRequest(SocketIO *sock);
	void HandleResponse(SocketIO *sock);
	void GetMethod(SocketIO *sock);
};