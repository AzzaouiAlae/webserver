#pragma once
#include "Headers.hpp"
#include "Socket.hpp"
#include "NetIO.hpp"
#include "Response.hpp"

#define HTTPLog(lvl) lvl("HTTPContext") << _logPrefix() << "HTTPContext, "

class HTTPContext : public IContext
{
	ClientSocket *_sock;
	int _status;
	Routing _router;
	Repsense _repsense;
	char *_buf;
	Multiplexer *_multiplexer;
	string _errNo;
	bool _isMaxBodyInit;
	int _readLen;
	vector<Config::Server> *_servers;

	void _handleRequest();
	void _readFromSocket();
	void _parseAndConfig();
	void _setMaxBodySize();
	void _handleConnectionEnd();
	string _prefix;
	string _logPrefix();
public:
	enum Status
	{
		eReadingRequest = 0,
		eWritingResponse = 1,
		eWriteError = 2,
		eDone = 3,
		eDelete = 4,
	};
	HTTPContext(vector<Config::Server> *servers, size_t maxBodySize, ClientSocket *sock);
	~HTTPContext();
	
	vector<Config::Server> *GetServers() const;
	void SetServers(vector<Config::Server> *servers);
	void Handle(AFd *fd);
	int GetStatus() const;
};
