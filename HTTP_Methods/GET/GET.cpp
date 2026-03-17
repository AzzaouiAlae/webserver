#include "GET.hpp"

GET::GET(SocketIO *sock, Routing *router): AMethod(sock, router)
{
	sendListFiles = 0;
	targetToSend = 0;
	sent = -1;
	filesListStr = "";
	DEBUG("GET") << "GET initialized, socket fd=" << sock->GetFd();
}

GET::~GET()
{}

bool GET::HandleResponse()
{
	DEBUG("GET") << "Socket fd: " << sock->GetFd() << ", GET::HandleResponse() start";

	ClientRequest &req = router->GetRequest();
	sock->SetStateByFd(sock->GetFd());
	string method = req.getMethod();
	ResolvePath();

	DDEBUG("GET") 
		<< "Socket fd: " << sock->GetFd()
		<< ", method=" << method
		<< ", readyToSend=" << readyToSend
		<< ", sendListFiles=" << sendListFiles
		<< ", emptyRoot=" << router->GetPath().emptyRoot();

	if (sock->isTimeOut()) 
	{
		if (router->GetPath().isCGI())
			HandelErrorPages("504");
		else
			HandelErrorPages("408");
		sock->UpdateTime();
		return del;
	}
	else if (!readyToSend && !IsMethodAllowed(method))
	{
		if (method == "GET" || method == "POST" || method == "DELETE" || method == "HEAD")
			HandelErrorPages("405");
		else 
			HandelErrorPages("501");
	}
	if (sendListFiles)
		SendListFilesResponse();
	else if (readyToSend)
		SendResponse();
	else if (method == "GET" || method == "HEAD")
		GetMethod();

	return del;
}

void GET::GetMethod()
{
	DDEBUG("GET") 
		<< "Socket fd: " << sock->GetFd()
		<< ", GetMethod: isRedir=" << router->GetPath().isRedirection()
		<< ", found=" << router->GetPath().isFound()
		<< ", hasPerm=" << router->GetPath().hasPermission()
		<< ", isDir=" << router->GetPath().isDirectory()
		<< ", isFile=" << router->GetPath().isFile()
		<< ", isCGI=" << router->GetPath().isCGI();
	if (router->GetPath().isRedirection() || router->GetPath().isRedirectionToDir())
		SendRedirection();
	else if (router->GetPath().isFound() == false)
		HandelErrorPages("404");
	else if (router->GetPath().hasPermission() == false)
		HandelErrorPages("403");
	else if (router->GetPath().isDirectory()) {
		int idxSrv = router->srv->autoindex, idxLoc = -1;
		if (router->loc) {
			idxLoc = router->loc->autoindex;
		}
		if ((idxSrv <= 0 && idxLoc == -1) || idxLoc == 0 )
			HandelErrorPages("403");
		else
			CreateListFilesResponse();
	}
	else if (router->GetPath().isCGI())
		HandelCGI();
	else if (router->GetPath().isFile())
		ServeFile();
}

void GET::OpenFile(const string &path)
{
	fileFd = open(path.c_str(), O_RDONLY | O_CLOEXEC);
}



void GET::PrepareFileResponse()
{
	bodySize = Utility::getFileSize(filename);
	ShouldSend = bodySize;
	code = "200";
	CreateResponseHeader();
	if ("HEAD" == router->GetRequest().getMethod())
		ShouldSend = 0;
	ShouldSend += responseHeaderStr.length();
	readyToSend = true;
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();
	MulObj->ChangeToEpollOut(sock);
}

void GET::ServeFile()
{
	DDEBUG("GET") << "Socket fd: " << sock->GetFd() << ", ServeFile: '" << filename << "'";
	OpenFile(filename);
	PrepareFileResponse();
}

void GET::GetStaticIndex()
{
	code = "200";
	staticFile = StaticFile::GetFileByName("index");
	bodySize = staticFile->GetSize();
	ShouldSend = bodySize;
	filename = ".html";
	CreateResponseHeader();
	if ("HEAD" == router->GetRequest().getMethod())
		ShouldSend = 0;
	ShouldSend += responseHeaderStr.length();
	readyToSend = true;
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();
	MulObj->ChangeToEpollOut(sock);
}

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
		entry << "false, size: '" << st.st_size << "', date: '";
	}

	time_t modTime = st.st_mtime;
	string s = ctime(&modTime);
	entry << s.substr(0, s.find("\n")) << "' },\n";

	return entry.str();
}

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
	if ("HEAD" == router->GetRequest().getMethod())
		ShouldSend = 0;
	ShouldSend += responseHeaderStr.length();

	sendListFiles = 1;
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();
	MulObj->ChangeToEpollOut(sock);
}

void GET::SendChunk(const char *data, int dataSize)
{
	int sent = sock->Send((void *)data, dataSize);
	if (sent > 0) {
		sended += sent;
		sock->UpdateTime();
	}
}

void GET::SendListFilesStr(const string &str)
{
	int len = targetToSend - sended;
	int offset = str.length() - len;

	const char *b = &(str[offset]);
	SendChunk(b, len);
}

void GET::SendAutoIndex(StaticFile *f)
{
	int len = targetToSend - sended;
	int offset = f->GetSize() - len;
	const char *b = &((f->GetData())[offset]);

	SendChunk(b, len);
}

void GET::AdvanceListFilesState()
{
	if (sended >= targetToSend)
		sendListFiles++;
	if (sendListFiles == 7 || Utility::SigPipe || sock->errorNumber == eWriteError)
		del = true;
}

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
