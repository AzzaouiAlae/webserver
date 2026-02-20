#include "Routing.hpp"

Routing::Routing()
{
	RequestComplete = false;
	srv = NULL;
}
ClientRequest &Routing::GetRequest()
{
	return request;
}

bool Routing::isRequestComplete()
{
	return RequestComplete;
}

void Routing::SetRequestComplete()
{
	RequestComplete = true;
}

Path &Routing::GetPath()
{
	return path;
}

string Routing::CreatePath(Config::Server *srv)
{
	if (strPath != "")
		return strPath;
	string h = request.getHost();
	this->srv = srv;
	
	path.CreatePath(*srv, request.getPath());
	strPath = path.getFullPath();
	return strPath;
}
