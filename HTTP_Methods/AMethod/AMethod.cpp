#include "AMethod.hpp"
#include "BuffersStrategy.hpp"
#include "FileStrategy.hpp"

map<string, string> AMethod::_statusMap;

AMethod::AMethod(SocketIO *sock, Routing *router)
{
	_cgi = NULL;
	_staticFile = NULL;
	_totalByteToSend = 0;
	_bodySize = 0;
	_sended = 0;
	_fileFd = -1;
	_statusCode = "";
	_filename = "";
	_responseHeaderStr = "";
	this->_sock = sock;
	this->_router = router;
	_initStatusMap();
	DEBUG("AMethod") << "AMethod initialized, socket fd=" << sock->GetFd();
	_status = eCreateResponse;
	_multiplexer = Multiplexer::GetCurrentMultiplexer();
	_sendStrategy = NULL;
	_readStrategy = NULL;
}

void AMethod::_createTimeoutResponse()
{
	if (_sended > 0)
	{
		_sock->closeConnection = true;
		_status = eComplete;
		return;
	}
	_buffers.clear();
	if (_router->GetPath().isCGI())
		HandelErrorPages("504");
	else
		HandelErrorPages("408");
	_sock->UpdateTime();
}

void AMethod::_resolvePath()
{
	_filename = _router->CreatePath(_router->srv);
	if (_router->GetRequest().getMethod() != "GET" &&
		_router->GetRequest().getMethod() != "HEAD")
	{
		_filename = _router->GetPath().OriginalPath.path;
	}
	DEBUG("AMethod")
		<< "Socket fd: " << _sock->GetFd()
		<< ", method: " << _router->GetRequest().getMethod()
		<< ", resolved path: " << _filename
		<< ", isCGI=" << _router->GetPath().isCGI()
		<< ", isDir=" << _router->GetPath().isDirectory()
		<< ", isFile=" << _router->GetPath().isFile()
		<< ", isRedir=" << _router->GetPath().isRedirection()
		<< ", found=" << _router->GetPath().isFound();
}

string AMethod::_escapeForJS(const string &input)
{
	string output;
	for (size_t i = 0; i < input.length(); ++i)
	{
		if (input[i] == '\'')
		{
			output += "\\'";
		}
		else if (input[i] == '\\')
		{
			output += "\\\\";
		}
		else
		{
			output += input[i];
		}
	}
	return output;
}

AMethod::~AMethod()
{
	if (_fileFd != -1)
	{
		close(_fileFd);
	}
	delete _cgi;
	if (_sendStrategy)
	{
		delete _sendStrategy;
	}
}

string AMethod::_createDate()
{
	char date[100];
	time_t now = time(0);
	struct tm tm = *gmtime(&now);
	strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S GMT", &tm);
	return date;
}

string AMethod::_resolveServerName()
{
	string serverName = "";
	if (_router->srv)
		serverName = _router->srv->serverName;
	if (serverName == "")
		serverName = _router->srv->listen[0];
	return serverName;
}

string AMethod::_resolveMimeType()
{
	return Singleton::GetMime()[Utility::GetFileExtension(_filename)];
}

void AMethod::_addAlowMethodsHeader()
{
	if (_statusCode == "405")
	{
		_responseHeader
			<< "Allow: ";
		if (_router->loc != NULL && _router->loc->allowMethodExists)
		{
			_responseHeader << _router->loc->allowMethods[0];
			for (size_t i = 1; i < _router->loc->allowMethods.size(); i++)
			{
				if (_router->loc->allowMethods[i] == "POST" && _router->GetRequest().getMethod() == "POST")
					continue;
				_responseHeader << ", " << _router->loc->allowMethods[i];
			}
		}
		else if (_router->srv != NULL && _router->srv->allowMethodExists)
		{
			_responseHeader << _router->srv->allowMethods[0];
			for (size_t i = 1; i < _router->srv->allowMethods.size(); i++)
			{
				if (_router->srv->allowMethods[i] == "POST" && _router->GetRequest().getMethod() == "POST")
					continue;
				_responseHeader << ", " << _router->srv->allowMethods[i];
			}
		}
		else
		{
			if (_router->GetRequest().getMethod() == "POST")
				_responseHeader << "GET, DELETE, HEAD";
			else
				_responseHeader << "GET, POST, DELETE, HEAD";
		}
		_responseHeader << "\r\n";
	}
}

void AMethod::_addCookiesHeader()
{
	Session *session = NULL;
	session = _router->GetRequest().getSession();
	if (session != NULL)
	{
		_addCookies(*session);
	}
}

void AMethod::_createResponseHeader()
{
	_responseHeader.str("");

	_responseHeader
		<< "HTTP/1.1 " << _statusCode << " " << _statusMap[_statusCode] << "\r\n"
		<< "Date: " << _createDate() << "\r\n"
		<< "Server: " << _resolveServerName() << "\r\n"
		<< "Content-Type: " << _resolveMimeType() << "\r\n"
		<< "Content-Length: " << _bodySize << "\r\n";
	if (_sock->closeConnection)
		_responseHeader << "Connection: close\r\n";
	_addAlowMethodsHeader();
	_addCookiesHeader();
	_responseHeader
		<< "X-Content-Type-Options: nosniff\r\n"
		<< "\r\n";

	_responseHeaderStr = _responseHeader.str();
	DDEBUG("AMethod") << "Socket fd: " << _sock->GetFd()
					  << ", CreateResponseHeader:\n"
					  << _responseHeaderStr;
	_insertPair(_responseHeaderStr, 0);
}

void AMethod::_addPair(StaticFile *f)
{
	if (f == NULL)
		return;
	char *data = (char *)f->GetData();
	size_t size = f->GetSize();
	_addPair(data, size);
}

void AMethod::_addPair(string &str)
{
	char *data = (char *)str.c_str();
	size_t size = str.length();
	_addPair(data, size);
}

void AMethod::_insertPair(string &str, int idx)
{
	char *data = (char *)str.c_str();
	size_t size = str.length();
	pair<char *, size_t> pairData = make_pair(data, size);
	if (idx < 0 || idx >= (int)_buffers.size())
		_buffers.push_back(pairData);
	else
		_buffers.insert(_buffers.begin() + idx, pairData);
}

void AMethod::_addPair(const string &str)
{
	char *data = (char *)str.c_str();
	size_t size = str.length();
	_addPair(data, size);
}

void AMethod::_addPair(const char *data, size_t size)
{
	char *dataPtr = (char *)data;
	_addPair(dataPtr, size);
}

void AMethod::_addPair(char *data, size_t size)
{
	pair<char *, size_t> pairData = make_pair(data, size);
	_addPair(pairData);
}

void AMethod::_addPair(pair<char *, size_t> &data)
{
	_buffers.push_back(data);
}

void AMethod::_sendDefaultRespense(const string &code)
{
	DDEBUG("AMethod") << "Socket fd: " << _sock->GetFd() << ", SendDefaultRespense: sending " << code;
	this->_statusCode = code;
	_bodySize = 0;
	_filename = ".html";
	_createResponseHeader();
	_totalByteToSend = _responseHeaderStr.length();
	_status = eSendResponse;
	_multiplexer->ChangeToEpollOut(_sock);
	_insertPair(_responseHeaderStr, 0);
}

void AMethod::_createRedirectionHeader(const string &redirCode, const string &redirLocation)
{
	_responseHeader.str("");

	_responseHeader
		<< "HTTP/1.1 " << redirCode << " " << _statusMap[redirCode] << "\r\n"
		<< "Location: " << redirLocation << "\r\n"
		<< "Date: " << _createDate() << "\r\n"
		<< "Content-Length: 0\r\n";
	// << "Connection: close\r\n";
	Session *session = NULL;
	session = _router->GetRequest().getSession();
	if (session != NULL)
	{
		_addCookies(*session);
	}
	_responseHeader
		<< "\r\n";

	_responseHeaderStr = _responseHeader.str();
	_insertPair(_responseHeaderStr, 0);
}

string AMethod::_resolveErrorFilePath(const string &errorCode)
{
	return Config::GetErrorPath(*_router->srv, errorCode);
}

bool AMethod::_openErrorFile(const string &path)
{
	_fileFd = open(path.c_str(), O_RDONLY | O_CLOEXEC);
	return (_fileFd != -1);
}

void AMethod::_loadStaticErrorFile(const string &errorCode)
{
	_staticFile = StaticFile::GetFileByName(errorCode.c_str());
	if (_staticFile == NULL)
	{
		_staticFile = StaticFile::GetFileByName("404");
		_statusCode = "404";
	}
	_bodySize = _staticFile->GetSize();
	_totalByteToSend = _bodySize;
	_filename = ".html";
	_addPair(_staticFile);
	_sendStrategy = new BuffersStrategy(_buffers, *_sock);
}

void AMethod::HandelErrorPages(const string &err)
{
	_sock->closeConnection = true;
	ERR() << "Client " << Socket::getRemoteName(_sock->GetFd()) << " error " << err << " " << _statusMap[err];
	DEBUG("AMethod") << "Socket fd: " << _sock->GetFd() << ", AMethod::HandelErrorPages, code=" << err;
	_statusCode = err;
	string path = _resolveErrorFilePath(err);
	bool isPath = (path != "");
	_filename = _router->srv->root + "/" + path;

	DDEBUG("AMethod") << "  -> errorFilePath='" << path << "', isPath=" << isPath << ", filename='" << _filename << "'";

	if (isPath && _openErrorFile(_filename))
	{
		_bodySize = Utility::getFileSize(_filename);
		_totalByteToSend = _bodySize;
		if ("HEAD" == _router->GetRequest().getMethod())
			_sendStrategy = new BuffersStrategy(_buffers, *_sock);
		else
			_sendStrategy = new FileStrategy(_buffers, _fileFd, _bodySize, *_sock);
		DDEBUG("AMethod") << "  -> Opened error file, bodySize=" << _bodySize;
	}
	else
	{
		DDEBUG("AMethod") << "  -> No custom error page, loading static fallback.";
		_loadStaticErrorFile(err);
	}
	_createResponseHeader();
	if ("HEAD" == _router->GetRequest().getMethod())
	{
		_totalByteToSend = 0;
	}
	_totalByteToSend += _responseHeaderStr.length();
	_multiplexer->ChangeToEpollOut(_sock);
	_status = AMethod::eSendResponse;
}

void AMethod::_sendResponse()
{
	const char *h;
	int size = 0, len;

	if (_staticFile != NULL)
	{
		if (_sended < (int)_responseHeaderStr.length())
		{
			h = &((_responseHeaderStr.c_str())[_sended]);
			len = _responseHeaderStr.length() - _sended;
			size = _sock->Send((void *)h, len);
		}
		else
		{
			len = _sended - (int)_responseHeaderStr.length();
			h = &((_staticFile->GetData())[len]);
			size = _sock->Send((void *)h, _staticFile->GetSize() - len);
		}
	}
	else if (_sended < (int)_responseHeaderStr.length())
	{
		h = &((_responseHeaderStr.c_str())[_sended]);
		len = _responseHeaderStr.length() - _sended;
		size = _sock->Send((void *)h, len);
	}
	else if (_sended >= (int)_responseHeaderStr.length())
	{
		size = _sock->FileToSocket(_fileFd, _totalByteToSend - _sended);
	}

	if (size > 0)
	{
		_sended += size;
		_sock->UpdateTime();
	}

	if (_totalByteToSend <= _sended || Utility::SigPipe || _sock->errorNumber == eWriteError)
	{
		if (_sock->errorNumber == eWriteError)
		{
			ERR() << "Error to write to socket";
			_sock->closeConnection = true;
		}
		_status = eComplete;
		_sock->cleanBody = true;
		INFO() << "Client " << Socket::getRemoteName(_sock->GetFd()) << " <- " << _statusCode << " " << _statusMap[_statusCode];
		DDEBUG("AMethod") << "Socket fd: " << _sock->GetFd() << ", SendResponse complete, code=" << _statusCode;
	}
	DDEBUG("AMethod") << "Socket fd: " << _sock->GetFd() << ", SendResponse: sent=" << size << ", progress=" << _sended << "/" << _totalByteToSend;
}

void AMethod::_handelCGI()
{
	_status = eCGIResponse;
	if (_cgi == NULL)
		_cgi = new Cgi(_router, _sock);
	try
	{
		_cgi->Handle();
		if (_cgi->isComplete())
		{
			_status = eComplete;
			_sock->cleanBody = true;
		}
	}
	catch (exception &e)
	{
		HandelErrorPages(e.what());
	}
}

void AMethod::_sendRedirection()
{
	Path &path = _router->GetPath();
	string redirCode = path.getRedirCode();
	string redirLoc = path.getRedirPath();

	DDEBUG("AMethod") << "Socket fd: " << _sock->GetFd()
					  << ", SendRedirection: code=" << redirCode << ", location='" << redirLoc << "'";

	_createRedirectionHeader(redirCode, redirLoc);
	DDEBUG("AMethod") << "\n"
					  << _responseHeaderStr;
	_totalByteToSend = _responseHeaderStr.length();

	_status = AMethod::eSendResponse;
	_multiplexer->ChangeToEpollOut(_sock);

	_sendStrategy = new BuffersStrategy(_buffers, *_sock);
}

bool AMethod::_isMethodAllowed(const string &method)
{
	bool allowed = true;
	if (method != "GET" && method != "POST" && method != "DELETE" && method != "HEAD")
		return false;
	vector<string>::iterator start, end;
	if (_router->loc != NULL && _router->loc->allowMethodExists)
	{
		start = _router->loc->allowMethods.begin();
		end = _router->loc->allowMethods.end();
	}
	else if (_router->srv != NULL && _router->srv->allowMethodExists)
	{
		start = _router->srv->allowMethods.begin();
		end = _router->srv->allowMethods.end();
	}
	else
		return true;
	allowed = (find(start, end, method) != end);
	DDEBUG("AMethod") << "Socket fd: " << _sock->GetFd() << ", IsMethodAllowed('" << method << "')=" << allowed;

	return allowed;
}

void AMethod::_executeStrategy(AStrategy *strategy)
{
	int status = strategy->Execute();
	if (status == AStrategy::eWriteError)
	{
		_sock->closeConnection = true;
		_status = eComplete;
	}
	else if (status == AStrategy::eOpenFileError)
		HandelErrorPages("403");
	else if   (status == AStrategy::eChunkedError
			|| status == AStrategy::eMultipartError
			|| status == AStrategy::eReadError)
		HandelErrorPages("400");
}

void AMethod::_initStatusMap()
{
	if (_statusMap.size() > 0)
		return;

	// ---- 1xx Informational ----
	_statusMap["100"] = "Continue";
	_statusMap["101"] = "Switching Protocols";
	_statusMap["102"] = "Processing";

	// ---- 2xx Success ----
	_statusMap["200"] = "OK";
	_statusMap["201"] = "Created";
	_statusMap["202"] = "Accepted";
	_statusMap["203"] = "Non-Authoritative Information";
	_statusMap["204"] = "No Content";
	_statusMap["205"] = "Reset Content";
	_statusMap["206"] = "Partial Content";

	// ---- 3xx Redirection ----
	_statusMap["300"] = "Multiple Choices";
	_statusMap["301"] = "Moved Permanently";
	_statusMap["302"] = "Found";
	_statusMap["303"] = "See Other";
	_statusMap["304"] = "Not Modified";
	_statusMap["305"] = "Use Proxy";
	_statusMap["307"] = "Temporary Redirect";
	_statusMap["308"] = "Permanent Redirect";

	// ---- 4xx Client Errors ----
	_statusMap["400"] = "Bad Request";
	_statusMap["401"] = "Unauthorized";
	_statusMap["403"] = "Forbidden";
	_statusMap["404"] = "Not Found";
	_statusMap["405"] = "Method Not Allowed";
	_statusMap["408"] = "Request Timeout";
	_statusMap["409"] = "Conflict";
	_statusMap["410"] = "Gone";
	_statusMap["411"] = "Length Required";
	_statusMap["413"] = "Payload Too Large";
	_statusMap["414"] = "URI Too Long";
	_statusMap["415"] = "Unsupported Media Type";
	_statusMap["429"] = "Too Many Requests";
	_statusMap["431"] = "Request Header Fields Too Large";

	// ---- 5xx Server Errors ----
	_statusMap["500"] = "Internal Server Error";
	_statusMap["501"] = "Not Implemented";
	_statusMap["502"] = "Bad Gateway";
	_statusMap["503"] = "Service Unavailable";
	_statusMap["504"] = "Gateway Timeout";
	_statusMap["505"] = "HTTP Version Not Supported";
}

void AMethod::_addCookies(Session &session)
{
	string Max_age = session.data["Max-Age"];
	string path = _router->GetRequest().getPath();
	map<string, string>::iterator it = session.data.begin();
	for (; it != session.data.end(); it++)
	{
		if (it->first != "Max-Age")
		{
			_responseHeader << "Set-Cookie:" << " " << it->first << "=" << it->second << ";" << " Max-Age=" << SESSION_TIMEOUT << ";" << " Path=" << path << "\r\n";
			DDEBUG("AddCookies") << "Set-Cookie:" << " " << it->first << "=" << it->second << ";" << " Max-Age=" << SESSION_TIMEOUT << ";" << " Path=" << path << "\r\n";
		}
	}
}
map<string, string> &AMethod::getStatusMap() { return (_statusMap); }
