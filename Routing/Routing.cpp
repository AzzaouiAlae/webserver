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
	string h = request.getHost().substr(1);
	int st = 0;
	srv =  Parsing::GetServerByName(h, st, *servers);
	if (srv == NULL)
		srv = Parsing::GetServerByHost(*servers, h);
	path.CreatePath(srv, request.getPath());
	strPath = path.getFullPath();
	return strPath;
}
