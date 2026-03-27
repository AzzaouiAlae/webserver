#pragma once
#include "Headers.hpp"
#include "Socket.hpp"
#include "SocketIO.hpp"
#include "SocketPipe.hpp"
#include "Response.hpp"

#define SAFE_MARGIN 1024 * 64
#define HTTPLog(lvl) lvl("HTTPContext") << _logPrefix() << "HTTPContext, "

class HTTPContext : public IContext
{
	enum Status
	{
		eReadingRequest = 0,
		eWritingResponse = 1,
		eWriteError = 2,
		eDone = 3,
		eDelete = 4,
	};
	SocketIO *_sock;
	Status _status;
	Routing _router;
	Repsense _repsense;
	char *_buf;
	Multiplexer *_multiplexer;
	AFd *_out;
	AFd *_in;
	string _errNo;
	bool _isMaxBodyInit;
	int _readLen;

	void _handleRequest();
	void _readFromSocket();
	void _parseAndConfig();
	void _setMaxBodySize();
	
	void _setupPipeline();
	void _activeInPipe();
	void _activeOutPipe();
	void _markedSocketToFree();
	
	string _logPrefix();
public:
	HTTPContext(vector<Config::Server> *servers, size_t maxBodySize, SocketIO *sock);
	~HTTPContext();
	vector<Config::Server> *servers;
	void Handle(AFd *fd);
};
