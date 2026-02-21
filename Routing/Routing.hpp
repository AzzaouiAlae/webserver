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
	DeprecatedConfig::DeprecatedServer *srv;
	DeprecatedConfig::DeprecatedServer::DeprecatedLocation *loc;
	ClientRequest &GetRequest();
	DeprecatedPath &GetPath();
	Routing();
	bool isRequestComplete();
	void SetRequestComplete();
	string CreatePath(DeprecatedConfig::DeprecatedServer *srv);
};

