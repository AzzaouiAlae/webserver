#pragma once
#include "Headers.hpp"
#include "Socket.hpp"
#include "SocketIO.hpp"
#include "SocketPipe.hpp"
#include "Response.hpp"

#define SAFE_MARGIN 1024 * 64
#define HTTPLog(lvl) lvl("HTTPContext") << logPrefix() << "HTTPContext, "

class HTTPContext : public IContext
{
	Repsense repsense;
	Routing router;
	SocketIO *sock;
	char *buf;
	AFd *out;
	AFd *in;
	bool err;
	string errNo;
	bool isMaxBodyInit;

    int  _readFromSocket();

    bool _parseAndConfig(int len);

    void _setupPipeline();
	string logPrefix();
public:
	void activeInPipe();
	void activeOutPipe();
	HTTPContext(vector<Config::Server > *servers, size_t maxBodySize, SocketIO *sock);
	~HTTPContext();
	vector<Config::Server > *servers;
	void Handle(AFd *fd);
	void HandleRequest();
	void MarkedSocketToFree();
	void setMaxBodySize();
	Routing &GetRouter() { return router; }
};
