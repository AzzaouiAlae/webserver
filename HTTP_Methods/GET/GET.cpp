#include "GET.hpp"

// ══════════════════════════════════════════════
//  Constructor / Destructor
// ══════════════════════════════════════════════

// Does one thing: initializes GET-specific members to default values
GET::GET(SocketIO *sock, Routing *router): AMethod(sock, router)
{
	sendListFiles = 0;
	targetToSend = 0;
	sent = -1;
	filesListStr = "";
	DEBUG("GET") << "GET initialized, socket fd=" << sock->GetFd();
}

// Does one thing: cleanup (base class ~AMethod handles fileFd)
GET::~GET()
{
}

// ══════════════════════════════════════════════
//  HandleResponse — GET entry point
// ══════════════════════════════════════════════

// Does one thing: dispatches to the correct sending/processing path for GET
bool GET::HandleResponse()
{
	DEBUG("GET") << "Socket fd: " << sock->GetFd() << ", GET::HandleResponse() start";

	ClientRequest &req = router->GetRequest();
	sock->SetStateByFd(sock->GetFd());
	string method = req.getMethod();
	ResolvePath();

	DDEBUG("GET") << "Socket fd: " << sock->GetFd()
				   << ", method=" << method
				   << ", readyToSend=" << readyToSend
				   << ", sendListFiles=" << sendListFiles
				   << ", emptyRoot=" << router->GetPath().emptyRoot();

	// Check allowed methods (shared logic from AMethod)
	if (!readyToSend && !IsMethodAllowed(method))
	{
		if (method == "GET" || method == "POST" || method == "DELETE")
		{
			DDEBUG("GET") << "Socket fd: " << sock->GetFd() << ", method '" << method << "' not allowed, sending 405.";
			HandelErrorPages("405");
		}
	}

	if (router->GetPath().emptyRoot()) {
		if (router->GetRequest().getPath() == "/") {
			DDEBUG("GET") << "Socket fd: " << sock->GetFd() << ", serving static index (empty root).";
			GetStaticIndex();
		}
		else
		{
			DDEBUG("GET") << "Socket fd: " << sock->GetFd() << ", empty root and path != '/', sending 404.";
			HandelErrorPages("404");
		}
	}

		
	else if (sendListFiles)
		SendListFilesResponse();
	else if (readyToSend)
	{
		
		SendResponse();
	}
	else if (method == "GET")
		GetMethod();
	else
		return true;

	return del;
}

// ══════════════════════════════════════════════
//  GET Dispatch
// ══════════════════════════════════════════════

// Does one thing: resolves the path, then dispatches based on path state
void GET::GetMethod()
{
	DDEBUG("GET") << "Socket fd: " << sock->GetFd()
				   << ", GetMethod: isRedir=" << router->GetPath().isRedirection()
				   << ", found=" << router->GetPath().isFound()
				   << ", hasPerm=" << router->GetPath().hasPermission()
				   << ", isDir=" << router->GetPath().isDirectory()
				   << ", isFile=" << router->GetPath().isFile()
				   << ", isCGI=" << router->GetPath().isCGI();
	if (router->GetPath().isRedirection())
		SendRedirection();
	else if (router->GetPath().isFound() == false)
		HandelErrorPages("404");
	else if (router->GetPath().hasPermission() == false)
		HandelErrorPages("403");
	else if (router->GetPath().isDirectory())
		CreateListFilesResponse();
	else if (router->GetPath().isFile())
		ServeFile();
	else if (router->GetPath().isCGI())
	{
		// CGI handling — to be implemented
	}

}

// ══════════════════════════════════════════════
//  File Serving (broken into 3 steps)
// ══════════════════════════════════════════════

// Does one thing: opens the file for reading
void GET::OpenFile(const string &path)
{
	fileFd = open(path.c_str(), O_RDONLY);
}

// Does one thing: sets body size, status code, and creates the response header
void GET::PrepareFileResponse()
{
	bodySize = Utility::getFileSize(filename);
	ShouldSend = bodySize;
	code = "200";
	CreateResponseHeader();
	ShouldSend += responseHeaderStr.length();
	readyToSend = true;
}

// Does one thing: orchestrates file serving (open → prepare → send)
void GET::ServeFile()
{
	DDEBUG("GET") << "Socket fd: " << sock->GetFd() << ", ServeFile: '" << filename << "'";
	OpenFile(filename);
	PrepareFileResponse();
	
	SendResponse();
}

// ══════════════════════════════════════════════
//  Static Index
// ══════════════════════════════════════════════

// Does one thing: loads the built-in static index page and prepares response
void GET::GetStaticIndex()
{
	code = "200";
	staticFile = StaticFile::GetFileByName("index");
	bodySize = staticFile->GetSize();
	ShouldSend = bodySize;
	filename = ".html";
	CreateResponseHeader();
	ShouldSend += responseHeaderStr.length();
	readyToSend = true;
	SendResponse();
}

// ══════════════════════════════════════════════
//  Directory Listing: Building
// ══════════════════════════════════════════════

// Does one thing: formats a single directory entry as a JS object string
string GET::FormatDirectoryEntry(const string &name, const struct stat &st, const string &requestPath)
{
	stringstream entry;

	entry << "{ name: '" << escapeForJS(name)
		  << "', href: '" << Path::encodePath(requestPath)
		  << (requestPath[requestPath.size() - 1] == '/' ? "" : "/")
		  << Path::encodePath(name)
		  << "', isDir: ";

	if (S_ISDIR(st.st_mode))
	{
		entry << "true, size: '-', date: '";
	}
	else
	{
		entry << "false, size: '"
			  << st.st_size
			  << "', date: '";
	}

	time_t modTime = st.st_mtime;
	string s = ctime(&modTime);
	entry << s.substr(0, s.find("\n")) << "' },\n";

	return entry.str();
}

// Does one thing: scans a directory and appends formatted entries to filesList
void GET::ListFiles(const string &path)
{
	DIR *dir = opendir(path.c_str());
	if (!dir)
		return;

	struct dirent *entry;
	string requestPath = router->GetRequest().getPath();

	while ((entry = readdir(dir)) != NULL)
	{
		string entryName = entry->d_name;

		if (entryName == "." || (entryName == ".." && requestPath == "/"))
			continue;

		string fullPath = path;
		if (fullPath[fullPath.size() - 1] != '/')
			fullPath += "/";
		fullPath += entryName;

		struct stat st;
		if (stat(fullPath.c_str(), &st) == 0)
			filesList << FormatDirectoryEntry(entryName, st, requestPath);
	}
	DDEBUG("GET") << "Socket fd: " << sock->GetFd() << ", ListFiles: scanned directory '" << path << "'";
	closedir(dir);
}

// Does one thing: calculates the total size of all auto-index template parts
int GET::CalculateAutoIndexSize()
{
	int size = 0;
	size += filesList.str().size();
	size += router->GetRequest().getPath().length();
	size += StaticFile::GetFileByName("autoIndex1")->GetSize();
	size += StaticFile::GetFileByName("autoIndex2")->GetSize();
	size += StaticFile::GetFileByName("autoIndex3")->GetSize();
	return size;
}

// Does one thing: orchestrates directory listing (scan → calculate → header → trigger send)
void GET::CreateListFilesResponse()
{
	DDEBUG("GET") << "Socket fd: " << sock->GetFd() << ", CreateListFilesResponse for path: '" << router->GetPath().getFullPath() << "'";
	ListFiles(router->GetPath().getFullPath());
	filesListStr = filesList.str();

	ShouldSend = CalculateAutoIndexSize();

	code = "200";
	filename = ".html";

	bodySize = ShouldSend;
	CreateResponseHeader();
	ShouldSend += responseHeaderStr.length();

	sendListFiles = 1;
	SendListFilesResponse();
}

// ══════════════════════════════════════════════
//  Directory Listing: Sending
// ══════════════════════════════════════════════

// Does one thing: sends a raw chunk from a buffer, updates sended counter
void GET::SendChunk(const char *data, int dataSize)
{
	int sent = sock->Send((void *)data, dataSize);
	if (sent > 0)
		sended += sent;
}

// Does one thing: calculates the correct offset into a string and sends that chunk
void GET::SendListFilesStr(const string &str)
{
	int len = targetToSend - sended;
	int offset = str.length() - len;

	

	const char *b = &(str[offset]);
	SendChunk(b, len);

	
}

// Does one thing: calculates the correct offset into a static file and sends that chunk
void GET::SendAutoIndex(StaticFile *f)
{
	int len = targetToSend - sended;
	int offset = f->GetSize() - len;
	const char *b = &((f->GetData())[offset]);

	SendChunk(b, len);

	
}

// Does one thing: advances the state machine counter and checks for completion
void GET::AdvanceListFilesState()
{
	if (sended >= targetToSend)
		sendListFiles++;
	if (sendListFiles == 7 || Utility::SigPipe)
		del = true;
}

// Does one thing: dispatches to the correct send function based on current state
void GET::SendListFilesResponse()
{
	int s1 = responseHeaderStr.length();
	int s2 = filesList.str().length();
	int s3 = router->GetRequest().getPath().length();
	int l1 = StaticFile::GetFileByName("autoIndex1")->GetSize();
	int l2 = StaticFile::GetFileByName("autoIndex2")->GetSize();
	int l3 = StaticFile::GetFileByName("autoIndex3")->GetSize();

	switch (sendListFiles)
	{
	case 1:
		targetToSend = s1;
		SendListFilesStr(responseHeaderStr);
		break;
	case 2:
		targetToSend = s1 + l1;
		SendAutoIndex(StaticFile::GetFileByName("autoIndex1"));
		break;
	case 3:
		targetToSend = s1 + l1 + s2;
		SendListFilesStr(filesListStr);
		break;
	case 4:
		targetToSend = s1 + l1 + s2 + l2;
		SendAutoIndex(StaticFile::GetFileByName("autoIndex2"));
		break;
	case 5:
		targetToSend = s1 + l1 + s2 + l2 + s3;
		SendListFilesStr(router->GetRequest().getPath());
		break;
	case 6:
		targetToSend = s1 + l1 + s2 + l2 + s3 + l3;
		SendAutoIndex(StaticFile::GetFileByName("autoIndex3"));
		break;
	}

	AdvanceListFilesState();
}
