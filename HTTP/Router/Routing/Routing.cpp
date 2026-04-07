#include "Routing.hpp"

Routing::Routing()
{
	_sendStrategy = NULL;
	_readStrategy = NULL;
	srv = NULL;
	loc = NULL;
	DEBUG("Routing") << "Routing initialized.";
}

Routing::~Routing()
{
	if (_sendStrategy)
	{
		delete _sendStrategy;
	}
	if (_readStrategy)
	{
		delete _readStrategy;
	}
}

ClientRequest &Routing::GetRequest()
{
	return _request;
}

Path &Routing::GetPath()
{
	return _path;
}

void Routing::AddPath(string &filename)
{
	if (filename.empty())
		filename = Utility::getRandomStr();
	filename = _path.OriginalPath.path + "/" + filename;
}

string Routing::CreatePath(Config::Server *srv)
{
	if (_strPath != "")
	{
		DDEBUG("Routing") << "CreatePath: returning cached path '" << _strPath << "'";
		return _strPath;
	}
	string h = _request.getHost();
	this->srv = srv;

	DEBUG("Routing") << "Creating path for request '" << _request.getPath() << "' on server '" << srv->serverName << "'";
	DDEBUG("Routing") << "  -> Host: '" << h << "', request method: '" << _request.getMethod() << "'";
	_path.CreatePath(*srv, _request.getPath(), _request.getMethod());
	_strPath = _path.getFullPath();
	DDEBUG("Routing")
		<< "  -> isCGI=" << _path.isCGI()
		<< ", isDir=" << _path.isDirectory()
		<< ", isFile=" << _path.isFile()
		<< ", isRedir=" << _path.isRedirection()
		<< ", found=" << _path.isFound();
	DEBUG("Routing") << "Path resolved: '" << _strPath << "'";
	loc = _path.getLocation();
	if (_path.isCGI())
	{
		string pathInfo;
		if (_path.getPathInfo() == "")
			pathInfo = _request.getPath();
		else
			pathInfo = _path.getPathInfo();
		_request.setUrlPart(_path.getCgiPath(), pathInfo);
	}
	return _strPath;
}

ChunkedData &Routing::GetChunkedData()
{
	return _chunkedData;
}

MultipartData &Routing::GetMultipartData()
{
	_multipartData.Initialize(_request.getMultipartBoundary());
	return _multipartData;
}

void Routing::SetSendStrategy(AStrategy *strategy) 
{ 
	if (_sendStrategy)
	{
		delete _sendStrategy;
	}
	_sendStrategy = strategy; 
}
void Routing::SetReadStrategy(AStrategy *strategy) 
{ 
	if (_readStrategy)
	{
		delete _readStrategy;
	}
	_readStrategy = strategy; 
}
AStrategy *Routing::GetSendStrategy() { return _sendStrategy; }
AStrategy *Routing::GetReadStrategy() { return _readStrategy; }