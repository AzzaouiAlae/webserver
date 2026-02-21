#pragma once
#include "../Headers.hpp"
#include <sys/sendfile.h>

class Routing 
{
	DeprecatedPath path;
	string strPath;
	ClientRequest request;
	bool RequestComplete;
public:
	Config::Server *srv;
	Config::Server::Location *loc;
	ClientRequest &GetRequest();
	DeprecatedPath &GetPath();
	Routing();
	bool isRequestComplete();
	void SetRequestComplete();
	string CreatePath(Config::Server *srv);
};

