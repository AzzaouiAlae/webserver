#include "AMethod.hpp"

map<string, string> AMethod::statusMap;

AMethod::AMethod(SocketIO *sock, Routing *router)
{
	_cgi = NULL;
	readyToSend = false;
	sendHeader = false;
	staticFile = NULL;
	ShouldSend = 0;
	bodySize = 0;
	del = false;
	sended = 0;
	fileFd = -1;
	code = "";
	filename = "";
	responseHeaderStr = "";
	this->sock = sock;
	this->router = router;
	InitStatusMap();
	pathResolved = false;
	DEBUG("AMethod") << "AMethod initialized, socket fd=" << sock->GetFd();
}

void AMethod::ResolvePath()
{
	if (pathResolved == false)
	{
		filename = router->CreatePath(router->srv);
		if (router->GetRequest().getMethod() != "GET" && 
			router->GetRequest().getMethod() != "HEAD")
		{
			filename = router->GetPath().OriginalPath.path;
		}
		pathResolved = true;
		DEBUG("AMethod") << "Socket fd: " << sock->GetFd()
						 << " " << router->GetRequest().getMethod()
						 << " resolved path: " << filename;
		DDEBUG("AMethod") << "  -> isCGI=" << router->GetPath().isCGI()
						  << ", isDir=" << router->GetPath().isDirectory()
						  << ", isFile=" << router->GetPath().isFile()
						  << ", isRedir=" << router->GetPath().isRedirection()
						  << ", found=" << router->GetPath().isFound();
	}
}

string AMethod::escapeForJS(const string &input)
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
	if (fileFd != -1)
	{
		close(fileFd);
	}
	delete _cgi;
}

string AMethod::CreateDate()
{
	char date[100];
	time_t now = time(0);
	struct tm tm = *gmtime(&now);
	strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S GMT", &tm);
	return date;
}

string AMethod::ResolveServerName()
{
	string serverName = "";
	if (router->srv)
		serverName = router->srv->serverName;
	if (serverName == "")
		serverName = router->srv->listen[0];
	return serverName;
}

string AMethod::ResolveMimeType()
{
	return Singleton::GetMime()[Utility::GetFileExtension(filename)];
}

void AMethod::addAlowMethodsHeader()
{
	if (code == "405")
	{
		responseHeader
			<< "Allow: ";
		if (router->loc != NULL && router->loc->allowMethodExists)
		{
			responseHeader << router->loc->allowMethods[0];
			for (size_t i = 1; i < router->loc->allowMethods.size(); i++) {
				if (router->loc->allowMethods[i] == "POST" && router->GetRequest().getMethod() == "POST")
					continue;
				responseHeader << ", " << router->loc->allowMethods[i];
			}
		}
		else if (router->srv != NULL && router->srv->allowMethodExists)
		{
			responseHeader << router->srv->allowMethods[0];
			for (size_t i = 1; i < router->srv->allowMethods.size(); i++) {
				if (router->srv->allowMethods[i] == "POST" && router->GetRequest().getMethod() == "POST")
					continue;
				responseHeader << ", " << router->srv->allowMethods[i];
			}
		}
		else
		{
			if (router->GetRequest().getMethod() == "POST")
				responseHeader << "GET, DELETE, HEAD";
			else
				responseHeader << "GET, POST, DELETE, HEAD";
		}
		responseHeader << "\r\n";
	}
}

void AMethod::addCookiesHeader()
{
	Session *session = NULL;
	session = router->GetRequest().getSession();
	if (session != NULL)
	{
		addCookies(*session);
	}
}

void AMethod::CreateResponseHeader()
{
	responseHeader.str("");

	responseHeader
		<< "HTTP/1.1 " << code << " " << statusMap[code] << "\r\n"
		<< "Date: " << CreateDate() << "\r\n"
		<< "Server: " << ResolveServerName() << "\r\n"
		<< "Content-Type: " << ResolveMimeType() << "\r\n"
		<< "Content-Length: " << bodySize << "\r\n";
	if (sock->closeConnection)
		responseHeader << "Connection: close\r\n";
	addAlowMethodsHeader();
	addCookiesHeader();
	responseHeader
		<< "X-Content-Type-Options: nosniff\r\n"
		<< "\r\n";

	responseHeaderStr = responseHeader.str();
	DDEBUG("AMethod") << "Socket fd: " << sock->GetFd()
					  << ", CreateResponseHeader:\n"
					  << responseHeaderStr;
}

void AMethod::SendDefaultRespense(const string &code)
{
	DDEBUG("AMethod") << "Socket fd: " << sock->GetFd() << ", SendDefaultRespense: sending 201 Created.";
	this->code = code;
	bodySize = 0;
	filename = ".html";
	CreateResponseHeader();
	ShouldSend = responseHeaderStr.length();
	readyToSend = true;
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();
	MulObj->ChangeToEpollOut(sock);
}

void AMethod::CreateRedirectionHeader(const string &redirCode, const string &redirLocation)
{
	responseHeader.str("");

	responseHeader
		<< "HTTP/1.1 " << redirCode << " " << statusMap[redirCode] << "\r\n"
		<< "Location: " << redirLocation << "\r\n"
		<< "Date: " << CreateDate() << "\r\n"
		<< "Content-Length: 0\r\n";
	// << "Connection: close\r\n";
	Session *session = NULL;
	session = router->GetRequest().getSession();
	if (session != NULL)
	{
		addCookies(*session);
	}
	responseHeader
		<< "\r\n";

	responseHeaderStr = responseHeader.str();
}

string AMethod::ResolveErrorFilePath(const string &errorCode)
{
	return Config::GetErrorPath(*router->srv, errorCode);
}

bool AMethod::OpenErrorFile(const string &path)
{
	fileFd = open(path.c_str(), O_RDONLY | O_CLOEXEC);
	return (fileFd != -1);
}

void AMethod::LoadStaticErrorFile(const string &errorCode)
{
	staticFile = StaticFile::GetFileByName(errorCode.c_str());
	if (staticFile == NULL)
	{
		staticFile = StaticFile::GetFileByName("404");
		code = "404";
	}
	bodySize = staticFile->GetSize();
	ShouldSend = bodySize;
	filename = ".html";
}

void AMethod::HandelErrorPages(const string &err)
{
	sock->closeConnection = true;
	ERR() << "Client " << Socket::getRemoteName(sock->GetFd()) << " error " << err << " " << statusMap[err];
	DEBUG("AMethod") << "Socket fd: " << sock->GetFd() << ", AMethod::HandelErrorPages, code=" << err;
	code = err;
	string path = ResolveErrorFilePath(err);
	bool isPath = (path != "");
	filename = router->srv->root + "/" + path;

	DDEBUG("AMethod") << "  -> errorFilePath='" << path << "', isPath=" << isPath << ", filename='" << filename << "'";

	if (isPath)
	{
		if (!OpenErrorFile(filename))
		{
			DDEBUG("AMethod") << "  -> Failed to open error file, loading static fallback.";
			LoadStaticErrorFile(err);
		}
		else
		{
			bodySize = Utility::getFileSize(filename);
			ShouldSend = bodySize;
			DDEBUG("AMethod") << "  -> Opened error file, bodySize=" << bodySize;
		}
	}
	else
	{
		DDEBUG("AMethod") << "  -> No custom error page, loading static fallback.";
		LoadStaticErrorFile(err);
	}
	CreateResponseHeader();
	if ("HEAD" == router->GetRequest().getMethod())
		ShouldSend = 0;
	ShouldSend += responseHeaderStr.length();
	readyToSend = true;
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();
	MulObj->ChangeToEpollOut(sock);
}

void AMethod::SendResponse()
{
	const char *h;
	int size = 0, len;

	if (staticFile != NULL)
	{
		if (sended < (int)responseHeaderStr.length())
		{
			h = &((responseHeaderStr.c_str())[sended]);
			len = responseHeaderStr.length() - sended;
			size = sock->Send((void *)h, len);
		}
		else
		{
			len = sended - (int)responseHeaderStr.length();
			h = &((staticFile->GetData())[len]);
			size = sock->Send((void *)h, staticFile->GetSize() - len);
		}
	}
	else if (sended < (int)responseHeaderStr.length())
	{
		h = &((responseHeaderStr.c_str())[sended]);
		len = responseHeaderStr.length() - sended;
		size = sock->Send((void *)h, len);
	}
	else if (sended >= (int)responseHeaderStr.length())
	{
		size = sock->FileToSocket(fileFd, ShouldSend - sended);
	}

	if (size > 0)
	{
		sended += size;
		sock->UpdateTime();
	}

	if (ShouldSend <= sended || Utility::SigPipe || sock->errorNumber == eWriteError)
	{
		if (sock->errorNumber == eWriteError)
			ERR() << "Error to write to socket";
		del = true;
		sock->cleanBody = true;
		INFO() << "Client " << Socket::getRemoteName(sock->GetFd()) << " <- " << code << " " << statusMap[code];
		DDEBUG("AMethod") << "Socket fd: " << sock->GetFd() << ", SendResponse complete, code=" << code;
	}
	DDEBUG("AMethod") << "Socket fd: " << sock->GetFd() << ", SendResponse: sent=" << size << ", progress=" << sended << "/" << ShouldSend;
}

void AMethod::HandelCGI()
{
	if (_cgi == NULL)
		_cgi = new Cgi(router, sock);
	try
	{
		_cgi->Handle();
		if (_cgi->isComplete())
		{
			del = true;
			sock->cleanBody = true;
		}
	}
	catch (exception &e)
	{
		HandelErrorPages(e.what());
	}
}

void _cgiResponse()
{
}

void AMethod::SendRedirection()
{
	Path &path = router->GetPath();
	string redirCode = path.getRedirCode();
	string redirLoc = path.getRedirPath();

	DDEBUG("AMethod") << "Socket fd: " << sock->GetFd()
					  << ", SendRedirection: code=" << redirCode << ", location='" << redirLoc << "'";

	CreateRedirectionHeader(redirCode, redirLoc);
	DDEBUG("AMethod") << "\n" << responseHeaderStr;
	ShouldSend = responseHeaderStr.length();
	readyToSend = true;
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();
	MulObj->ChangeToEpollOut(sock);
}

bool AMethod::IsMethodAllowed(const string &method)
{
	bool allowed = true;
	if (method != "GET" && method != "POST" && method != "DELETE" && method != "HEAD")
		return false;
	vector<string>::iterator start, end;
	if (router->loc != NULL && router->loc->allowMethodExists)
	{
		start = router->loc->allowMethods.begin();
		end = router->loc->allowMethods.end();
	}
	else if (router->srv != NULL && router->srv->allowMethodExists)
	{
		start = router->srv->allowMethods.begin();
		end = router->srv->allowMethods.end();
	}
	else
		return true;
	allowed = (find(start, end, method) != end);
	DDEBUG("AMethod") << "Socket fd: " << sock->GetFd() << ", IsMethodAllowed('" << method << "')=" << allowed;

	return allowed;
}

void AMethod::InitStatusMap()
{
	if (statusMap.size() > 0)
		return;

	// ---- 1xx Informational ----
	statusMap["100"] = "Continue";
	statusMap["101"] = "Switching Protocols";
	statusMap["102"] = "Processing";

	// ---- 2xx Success ----
	statusMap["200"] = "OK";
	statusMap["201"] = "Created";
	statusMap["202"] = "Accepted";
	statusMap["203"] = "Non-Authoritative Information";
	statusMap["204"] = "No Content";
	statusMap["205"] = "Reset Content";
	statusMap["206"] = "Partial Content";

	// ---- 3xx Redirection ----
	statusMap["300"] = "Multiple Choices";
	statusMap["301"] = "Moved Permanently";
	statusMap["302"] = "Found";
	statusMap["303"] = "See Other";
	statusMap["304"] = "Not Modified";
	statusMap["305"] = "Use Proxy";
	statusMap["307"] = "Temporary Redirect";
	statusMap["308"] = "Permanent Redirect";

	// ---- 4xx Client Errors ----
	statusMap["400"] = "Bad Request";
	statusMap["401"] = "Unauthorized";
	statusMap["403"] = "Forbidden";
	statusMap["404"] = "Not Found";
	statusMap["405"] = "Method Not Allowed";
	statusMap["408"] = "Request Timeout";
	statusMap["409"] = "Conflict";
	statusMap["410"] = "Gone";
	statusMap["411"] = "Length Required";
	statusMap["413"] = "Payload Too Large";
	statusMap["414"] = "URI Too Long";
	statusMap["415"] = "Unsupported Media Type";
	statusMap["429"] = "Too Many Requests";
	statusMap["431"] = "Request Header Fields Too Large";

	// ---- 5xx Server Errors ----
	statusMap["500"] = "Internal Server Error";
	statusMap["501"] = "Not Implemented";
	statusMap["502"] = "Bad Gateway";
	statusMap["503"] = "Service Unavailable";
	statusMap["504"] = "Gateway Timeout";
	statusMap["505"] = "HTTP Version Not Supported";
}

void AMethod::addCookies(Session &session)
{
	string Max_age = session.data["Max-Age"];
	string path = router->GetRequest().getPath();
	map<string, string>::iterator it = session.data.begin();
	for (; it != session.data.end(); it++)
	{
		if (it->first != "Max-Age")
		{

			responseHeader << "Set-Cookie:" << " " << it->first << "=" << it->second << ";" << " Max-Age=" << SESSION_TIMEOUT << ";" << " Path=" << path << "\r\n";
			DDEBUG("AddCookies") << "Set-Cookie:" << " " << it->first << "=" << it->second << ";" << " Max-Age=" << SESSION_TIMEOUT << ";" << " Path=" << path << "\r\n";
		}
	}
}
map<string, string> &AMethod::getStatusMap() { return (statusMap); }
