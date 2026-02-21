#include "Routing.hpp"

Routing::Routing()
{
	RequestComplete = false;
	srv = NULL;
	DEBUG("Routing") << "Routing initialized.";
}
ClientRequest &Routing::GetRequest()
{
	return request;
}

bool Routing::isRequestComplete()
{
	DDEBUG("Routing") << "isRequestComplete() = " << RequestComplete;
	return RequestComplete;
}

void Routing::SetRequestComplete()
{
	RequestComplete = true;
	DEBUG("Routing") << "Request marked as complete.";
}

DeprecatedPath &Routing::GetPath()
{
	return path;
}

string Routing::CreatePath(Config::Server *srv)
{
	if (strPath != "")
	{
		DDEBUG("Routing") << "CreatePath: returning cached path '" << strPath << "'";
		return strPath;
	}
	string h = request.getHost();
	this->srv = srv;
	
	DEBUG("Routing") << "Creating path for request '" << request.getPath() << "' on server '" << srv->serverName << "'";
	DDEBUG("Routing") << "  -> Host: '" << h << "', request method: '" << request.getMethod() << "'";
	path.CreatePath(*srv, request.getPath());
	strPath = path.getFullPath();
	DDEBUG("Routing") << "  -> isCGI=" << path.isCGI()
					   << ", isDir=" << path.isDirectory()
					   << ", isFile=" << path.isFile()
					   << ", isRedir=" << path.isRedirection()
					   << ", found=" << path.isFound();
	DEBUG("Routing") << "Path resolved: '" << strPath << "'";
	return strPath;
}
