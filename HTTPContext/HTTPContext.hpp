#pragma once
#include "../Headers.hpp"
#include "../Socket/Socket.hpp"
#include "../SocketIO/SocketIO.hpp"
#include "../Pipe/Pipe.hpp"
#include "../Repsense/Repsense.hpp"

#define BUF_SIZE (1024 * 1024 * 10)

class HTTPContext : public IContext
{
	Repsense repsense;
	Routing router;
	SocketIO *sock;
	char *buf;
	AFd *out;
	AFd *in;
public:
	void HandelErrorPages(const string& code);
	void activeInPipe();
	void activeOutPipe();
	HTTPContext();
	~HTTPContext();
	vector<AST<string> > *servers;
	void Handle(AFd *fd);
	void Handle(Socket *sock);
	void Handle();
	void HandleRequest();
	void MarkedSocketToFree();
	
};