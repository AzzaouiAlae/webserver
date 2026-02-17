#include "Post.hpp"

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

	Logging::Debug() << "Socket fd: " << sock->GetFd()
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
//  Success Response (201 Created)
// ══════════════════════════════════════════════

// Does one thing: builds and sends a 201 Created response
void Post::createPostResponse()
{
	code = "201";
	bodySize = 0;
	filename = ".html";
	CreateResponseHeader();
	ShouldSend = responseHeaderStr.length();
	readyToSend = true;
	SendResponse();
}
