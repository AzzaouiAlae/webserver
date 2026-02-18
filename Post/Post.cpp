#include "Post.hpp"
#include "../Pipe/Pipe.hpp"
#include "../HTTPContext/HTTPContext.hpp"

// ══════════════════════════════════════════════
//  Constructor / Destructor
// ══════════════════════════════════════════════



Post::Post(SocketIO *sock, Routing *router) : AMethod(sock, router)
{
	uploadFd = -1;
	readyToUpload = false;
	pathResolved = false;
	contentBodySize = 0;
	uploadedSize = 0;
	buf = (char *)calloc(1, BUF_SIZE);
}


Post::~Post()
{
	if (uploadFd != -1)
		close(uploadFd);
}

// ══════════════════════════════════════════════
//  HandleResponse — POST entry point
// ══════════════════════════════════════════════

// Does one thing: dispatches to the correct state for POST
bool Post::HandleResponse()
{

	sock->SetStateByFd(sock->GetFd());

	Logging::Debug() << "Socket fd: " << sock->GetFd() << " try to Handle POST Response";

	// 1. Already sending response (success or error) → keep sending
	if (readyToSend)
	{
		SendResponse();
		return del;
	}

	// 2. Currently uploading file to disk → continue writing
	if (readyToUpload)
	{
		uploadFileToDisk();
		return del;
	}

	// 3. Check method is allowed
	if (!IsMethodAllowed("POST"))
	{
		HandelErrorPages("405");
		return del;
	}

	// 4. Resolve path (only once)
	ResolvePath();

	// 5. Redirection
	if (router->GetPath().isRedirection())
	{
		SendRedirection();
		return del;
	}

	// 6. File already exists → 409
	if (router->GetPath().isFound() && router->GetPath().isFile())
	{
		HandelErrorPages("409");
		return del;
	}

	// 7. CGI (to be implemented)
	if (router->GetPath().isCGI())
	{
		// TODO: forward body to CGI via stdin
		return del;
	}

	// 8. Normal upload
	PostMethod();
	return del;
}




// ═══���══════════════════════════════════════════
//  Path Resolution
// ══════════════════════════════════════════════

// Does one thing: resolves file path from the request URL (only once)
void Post::ResolvePath()
{
	if (!pathResolved)
	{
		filename = router->CreatePath(router->srv);
		pathResolved = true;
		Logging::Debug() << "Socket fd: " << sock->GetFd()
						 << " POST resolved path: " << filename;
	}
}

// ══════════════════════════════════════════════
//  File Creation
// ══════════════════════════════════════════════

// Does one thing: creates/opens the upload file for writing
void Post::OpenUploadFile()
{
	uploadFd = open(filename.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
}

// ══════════════════════════════════════════════
//  POST Dispatch
// ══════════════════════════════════════════════

// Does one thing: validates location and starts the upload
void Post::PostMethod()
{
	int idx = Config::GetLocationIndex(*(router->srv), router->GetRequest().getPath());

	if (idx == -1)
	{
		HandelErrorPages("403");
		return;
	}

	OpenUploadFile();
	if (uploadFd == -1)
	{
		HandelErrorPages("403");
		return;
	}

	
    Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();

    MulObj->ChangeToEpollOut(sock);

	contentBodySize = router->GetRequest().getContentLen();
	readyToUpload = true;
	uploadFileToDisk();
}

// ══════════════════════════════════════════════
//  File Upload (writing body to disk)
// ══════════════════════════════════════════════

// Does one thing: writes buffered body data from memory to the file
void Post::WriteBodyFromMemory()
{
	string &body = router->GetRequest().getBody();
	if (body.length() <= uploadedSize)
		return;

	const char *data = &(body[uploadedSize]);
	int toWrite = body.length() - uploadedSize;
	int written = write(uploadFd, data, toWrite);

	if (written > 0)
		uploadedSize += written;
}

// Does one thing: splices data directly from socket to file (zero-copy)
void Post::WriteBodyFromSocket()
{
	int len = 0, w;
	int size = read(sock->GetFd(), buf, BUF_SIZE -1);
	if (errno == EAGAIN)
		Logging::Debug() << "EAGAIN	No data yet (non-blocking)	Retry on next epoll event";
	if (errno == EINTR)
		Logging::Debug() << "EINTR	Signal interrupted	Retry immediately or on next event";
	if (errno == ECONNRESET)
		Logging::Debug() << "ECONNRESET	Client killed connection	Set err = true, cleanup";
	if (errno == EBADF)
		Logging::Debug() << "EBADF	fd already closed (bug)	Set err = true, debug your code";
	if (errno == EFAULT)
		Logging::Debug() << "EFAULT	buf is bad pointer	Crash/set err = true";
	Logging::Debug() << "WriteBodyFromSocket size: " << size;
	if (size > 0) 
	{
		while(size - len > 0)
		{
			w = write(uploadFd, &(buf[len]), size - len);
			if (w < 0) 
			{
				HandelErrorPages("403");
				return;
			}
			len += w;
			Logging::Debug() << "write " << w << " from " << size;
		}
		uploadedSize += size;
	}
	usleep(300000);
}

// Does one thing: orchestrates writing — memory first, then socket
void Post::uploadFileToDisk()
{
	string &body = router->GetRequest().getBody();

	// First: write whatever the Request already buffered in memory
	if (body.length() > uploadedSize)
	{
		WriteBodyFromMemory();
	}
	// Then: if more data expected, read directly from socket to file
	else if (uploadedSize < contentBodySize)
	{
		WriteBodyFromSocket();
	}

	Logging::Debug() << "Socket fd: " << sock->GetFd()
					 << " POST uploaded " << uploadedSize
					 << " / " << contentBodySize << " bytes";

	// Check if upload is complete
	if (uploadedSize >= contentBodySize)
	{
		readyToUpload = false;
		createPostResponse();
	}
}

// ══════════════════════════════════════════════
//  Success Response
// ══════════════════════════════════════════════

// Does one thing: gets the return directive from the matched location (if any)
bool Post::GetLocationReturn(string &retCode, string &retBody)
{
	int idx = router->GetPath().matchedLocationIndex;
	if (idx == -1)
		return false;

	const Config::Server::Location &loc = router->srv->Locations[idx];
	if (loc.returnCode.empty())
		return false;

	retCode = loc.returnCode;
	retBody = loc.returnArg;
	return true;
}

// Does one thing: sends a redirection response (301, 302)
void Post::SendPostRedirection(const string &retCode, const string &retBody)
{
	CreateRedirectionHeader(retCode, retBody);
	Logging::Debug()	<< "Socket fd: " << sock->GetFd()
						<< " POST redirect " << retCode << " to " << retBody;
	ShouldSend = responseHeaderStr.length();
	readyToSend = true;
	SendResponse();
}

// Does one thing: sends a custom body response (e.g., return 200 '{"status":"ok"}')
void Post::SendPostCustomBody(const string &retCode, const string &retBody)
{
	code = retCode;
	bodySize = retBody.length();
	filename = ".json";
	CreateResponseHeader();
	responseHeaderStr += retBody;
	ShouldSend = responseHeaderStr.length();
	readyToSend = true;
	SendResponse();
}

// Does one thing: sends the default 201 Created (no body)
void Post::SendPostDefault()
{
	code = "201";
	bodySize = 0;
	filename = ".html";
	CreateResponseHeader();
	ShouldSend = responseHeaderStr.length();
	readyToSend = true;
	SendResponse();
}

// Does one thing: orchestrates which response to send after upload completes
void Post::createPostResponse()
{
	string retCode, retBody;

	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();

    MulObj->ChangeToEpollOut(sock);

	if (GetLocationReturn(retCode, retBody))
	{
		// Location has a "return" directive
		if (retCode == "301" || retCode == "302")
		{
			// return 301 /new-path;  → redirect
			SendPostRedirection(retCode, retBody);
		}
		else
		{
			// return 200 '{"status": "success"}';  → custom body
			SendPostCustomBody(retCode, retBody);
		}
	}
	else
	{
		// No "return" directive → default 201 Created
		SendPostDefault();
	}
}
