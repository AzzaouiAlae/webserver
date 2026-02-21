#include "Post.hpp"

// ══════════════════════════════════════════════
//  Constructor / Destructor
// ══════════════════════════════════════════════

Post::Post(SocketIO *sock, Routing *router) : AMethod(sock, router)
{
	uploadFd = -1;
	readyToUpload = false;
	contentBodySize = 0;
	uploadedSize = 0;
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();
    MulObj->ChangeToEpollIn(sock);
	DEBUG("Post") << "Post initialized, socket fd=" << sock->GetFd();
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

	DEBUG("Post") << "Socket fd: " << sock->GetFd() << ", Post::HandleResponse() start";
	DDEBUG("Post") << "Socket fd: " << sock->GetFd()
					<< ", readyToSend=" << readyToSend
					<< ", readyToUpload=" << readyToUpload
					<< ", uploadedSize=" << uploadedSize
					<< ", contentBodySize=" << contentBodySize;

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
		DDEBUG("Post") << "Socket fd: " << sock->GetFd() << ", POST method not allowed, sending 405.";
		HandelErrorPages("405");
		return del;
	}

	// 4. Resolve path (only once)
	ResolvePath();

	// 5. Redirection
	if (router->GetPath().isRedirection())
	{
		DDEBUG("Post") << "Socket fd: " << sock->GetFd() << ", redirection detected.";
		SendRedirection();
		return del;
	}

	// 6. File already exists → 409
	if (router->GetPath().isFound() && router->GetPath().isFile())
	{
		DDEBUG("Post") << "Socket fd: " << sock->GetFd() << ", file already exists, sending 409.";
		HandelErrorPages("409");
		return del;
	}

	// 7. CGI (to be implemented)
	if (router->GetPath().isCGI())
	{
		DDEBUG("Post") << "Socket fd: " << sock->GetFd() << ", CGI path detected (not yet implemented).";
		// TODO: forward body to CGI via stdin
		return del;
	}

	// 8. Normal upload
	PostMethod();
	return del;
}


// ══════════════════════════════════════════════
//  File Creation
// ══════════════════════════════════════════════

// Does one thing: creates/opens the upload file for writing
void Post::OpenUploadFile()
{
	uploadFd = open(filename.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
	DDEBUG("Post") << "Socket fd: " << sock->GetFd() << ", OpenUploadFile: '" << filename << "', fd=" << uploadFd;
}

// ══════════════════════════════════════════════
//  POST Dispatch
// ══════════════════════════════════════════════

// Does one thing: validates location and starts the upload
void Post::PostMethod()
{
	if (router->GetPath().getLocation() == NULL)
	{
		DDEBUG("Post") << "Socket fd: " << sock->GetFd() << ", PostMethod: no location matched, sending 403.";
		HandelErrorPages("403");
		return;
	}

	OpenUploadFile();
	if (uploadFd == -1)
	{
		DDEBUG("Post") << "Socket fd: " << sock->GetFd() << ", PostMethod: failed to open upload file, sending 403.";
		HandelErrorPages("403");
		return;
	}

	contentBodySize = router->GetRequest().getcontentlen();
	readyToUpload = true;
	DDEBUG("Post") << "Socket fd: " << sock->GetFd() << ", PostMethod: starting upload, contentBodySize=" << contentBodySize;
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
	int written = sock->SocketToFile(uploadFd, contentBodySize - uploadedSize);
	if (written > 0)
		uploadedSize += written;
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

	DEBUG("Post") << "Socket fd: " << sock->GetFd()
				 << " POST uploaded " << uploadedSize
				 << " / " << contentBodySize << " bytes";

	// Check if upload is complete
	if (uploadedSize >= contentBodySize)
	{
		close(uploadFd);
		uploadFd = -1;
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
	const Config::Server::Location *loc = router->GetPath().getLocation();
	if (loc == NULL || loc->returnCode.empty())
		return false;

	retCode = loc->returnCode;
	retBody = loc->returnArg;
	return true;
}

// Does one thing: sends a redirection response (301, 302)
void Post::SendPostRedirection(const string &retCode, const string &retBody)
{
	CreateRedirectionHeader(retCode, retBody);
	DEBUG("Post") << "Socket fd: " << sock->GetFd()
				  << " POST redirect " << retCode << " to " << retBody;
	ShouldSend = responseHeaderStr.length();
	readyToSend = true;
	SendResponse();
}

// Does one thing: sends a custom body response (e.g., return 200 '{"status":"ok"}')
void Post::SendPostCustomBody(const string &retCode, const string &retBody)
{
	DDEBUG("Post") << "Socket fd: " << sock->GetFd()
					<< ", SendPostCustomBody: code=" << retCode << ", bodyLen=" << retBody.length();
	code = retCode;
	bodySize = retBody.length();
	filename = ".json";
	CreateResponseHeader();
	responseHeaderStr += retBody;
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
		DDEBUG("Post") << "Socket fd: " << sock->GetFd()
						<< ", createPostResponse: location return code=" << retCode;
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
		DDEBUG("Post") << "Socket fd: " << sock->GetFd()
						<< ", createPostResponse: no return directive, sending 201 Created.";
		// No "return" directive → default 201 Created
		SendDefaultRespense();
	}
}