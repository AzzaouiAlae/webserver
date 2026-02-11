#include "Routing.hpp"

Routing::Routing()
{
	RequestComplete = false;
}
Request &Routing::GetRequest()
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

string Routing::CreatePath(vector<AST<string> > *servers)
{
	if (strPath != "")
		return strPath;

	srv = Parsing::GetServerByHost(*servers, request.getHost() + ":" + request.getport());
	path.CreatePath(srv, request.getPath());
	strPath = path.getFullPath();
	return strPath;
}