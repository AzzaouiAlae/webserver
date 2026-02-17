#include "AMethod.hpp"

map<string, string> AMethod::statusMap;

// ══════════════════════════════════════════════
//  Constructor / Destructor
// ══════════════════════════════════════════════

// Does one thing: initializes all member variables to default values
AMethod::AMethod(SocketIO *sock, Routing *router)
{
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
}

// Does one thing: closes the file descriptor if it was opened
AMethod::~AMethod()
{
	if (fileFd != -1)
	{
		close(fileFd);
	}
}


// ══════════════════════════════════════════════
//  Date Formatting
// ══════════════════════════════════════════════

// Does one thing: returns the current time formatted as an HTTP date string
string AMethod::CreateDate()
{
	char date[100];
	time_t now = time(0);
	struct tm tm = *gmtime(&now);
	strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S GMT", &tm);
	return date;
}

// ══════════════════════════════════════════════
//  Response Header Building (broken into 3 steps)
// ══════════════════════════════════════════════

// Does one thing: resolves the server name with fallback to listen address
string AMethod::ResolveServerName()
{
	string serverName = "";
	if (router->srv)
		serverName = router->srv->serverName;
	if (serverName == "")
		serverName = router->srv->listen[0];
	return serverName;
}

// Does one thing: resolves the MIME type based on the current filename extension
string AMethod::ResolveMimeType()
{
	return Singleton::GetMime()[Utility::GetFileExtension(filename)];
}

// Does one thing: assembles the full HTTP response header string
void AMethod::CreateResponseHeader()
{
	responseHeader.str("");

	responseHeader
		<< "HTTP/1.1 " << code << " " << statusMap[code] << "\r\n"
		<< "Date: " << CreateDate() << "\r\n"
		<< "Server: " << ResolveServerName() << "\r\n"
		<< "Content-Type: " << ResolveMimeType() << "\r\n"
		<< "Content-Length: " << bodySize << "\r\n"
		<< "Connection: close\r\n"
		<< "X-Content-Type-Options: nosniff\r\n"
		<< "\r\n";

	responseHeaderStr = responseHeader.str();
}

// Does one thing: builds a redirection-specific header (no body, includes Location)
void AMethod::CreateRedirectionHeader(const string &redirCode, const string &redirLocation)
{
	responseHeader.str("");

	responseHeader
		<< "HTTP/1.1 " << redirCode << " " << statusMap[redirCode] << "\r\n"
		<< "Location: " << redirLocation << "\r\n"
		<< "Date: " << CreateDate() << "\r\n"
		<< "Content-Length: 0\r\n"
		<< "Connection: close\r\n"
		<< "\r\n";

	responseHeaderStr = responseHeader.str();
}

// ══════════════════════════════════════════════
//  Error Handling (broken into 4 steps)
// ══════════════════════════════════════════════

// Does one thing: returns the configured error page path for the given code (or "")
string AMethod::ResolveErrorFilePath(const string &errorCode)
{
	return Config::GetErrorPath(*router->srv, errorCode);
}

// Does one thing: tries to open the error file, sets fileFd, returns true on success
bool AMethod::OpenErrorFile(const string &path)
{
	fileFd = open(path.c_str(), O_RDONLY);
	return (fileFd != -1);
}

// Does one thing: loads a static fallback error file (with 404 fallback if not found)
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

// Does one thing: orchestrates error page resolution, then prepares for sending
void AMethod::HandelErrorPages(const string &err)
{
	code = err;
	string path = ResolveErrorFilePath(err);
	bool isPath = (path != "");
	filename = router->srv->root + "/" + path;

	if (isPath)
	{
		if (!OpenErrorFile(filename))
		{
			LoadStaticErrorFile(err);
			return;
		}
		bodySize = Utility::getFileSize(filename);
		ShouldSend = bodySize;
	}
	else
	{
		LoadStaticErrorFile(err);
	}
	CreateResponseHeader();
	ShouldSend += responseHeaderStr.length();
	readyToSend = true;
	SendResponse();
}

// ══════════════════════════════════════════════
//  Sending Response
// ══════════════════════════════════════════════

// Does one thing: sends the next chunk of data (header + body) to the socket
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
		size = sock->sendFileWithHeader(h, len, fileFd, ShouldSend - sended);
	}
	else if (sended >= (int)responseHeaderStr.length())
	{
		size = sock->FileToSocket(fileFd, ShouldSend - sended);
	}

	if (size > 0)
		sended += size;

	Logging::Debug() << "Socket fd: " << sock->GetFd()
					 << " Send " << sended << " byte from " << ShouldSend;

	if (ShouldSend <= sended || Utility::SigPipe)
		del = true;
}

// ══════════════════════════════════════════════
//  Redirection
// ══════════════════════════════════════════════

// Does one thing: prepares and sends a redirection response
void AMethod::SendRedirection()
{
	Path &path = router->GetPath();
	string redirCode = path.getRedirCode();
	string redirLoc = path.getRedirPath();

	CreateRedirectionHeader(redirCode, redirLoc);

	Logging::Debug() << "Sending Redirection: " << redirCode << " to " << redirLoc;

	ShouldSend = responseHeaderStr.length();
	readyToSend = true;
	SendResponse();
}

// ══════════════════════════════════════════════
//  Method Validation
// ══════════════════════════════════════════════

// Does one thing: checks if the given method is in the server's allowed methods list
bool AMethod::IsMethodAllowed(const string &method)
{
	if (!router->srv->allowMethodExists)
		return true;

	vector<string>::iterator start = router->srv->allowMethods.begin();
	vector<string>::iterator end = router->srv->allowMethods.end();

	return (find(start, end, method) != end);
}

// ══════════════════════════════════════════════
//  Status Map Initialization
// ══════════════════════════════════════════════

// Does one thing: populates the static status code → reason phrase map
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