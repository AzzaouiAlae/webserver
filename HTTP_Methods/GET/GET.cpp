#include "GET.hpp"
#include "BuffersStrategy.hpp"
#include "FileStrategy.hpp"

GET::GET(ClientSocket *sock, Routing *router) : AMethod(sock, router)
{
	_filesListStr = "";
	DEBUG("GET") << "GET initialized, socket fd=" << sock->GetFd();
}

GET::~GET()
{
}

bool GET::HandleResponse()
{
	if (_sock->IsTimeOut())
		_createTimeoutResponse();
	if (_status == AMethod::eSendResponse)
	{
		_executeStrategy(_router->GetSendStrategy());
		if (_router->GetSendStrategy()->GetStatus() == AStrategy::eComplete)
			_status = eComplete;
	}
	else if (_status == GET::eInit)
		_createResponse();
	else if (_status == GET::eCGIResponse)
		_handleCGI();
	return _status == GET::eComplete;
}

void GET::_createResponse()
{
	DEBUG("GET") << "Socket fd: " << _sock->GetFd() << ", GET::HandleResponse() start";

	ClientRequest &req = _router->GetRequest();
	string method = req.getMethod();
	_resolvePath();

	DDEBUG("GET")
		<< "Socket fd: " << _sock->GetFd()
		<< ", GetMethod: isRedir=" << _router->GetPath().isRedirection()
		<< ", found=" << _router->GetPath().isFound()
		<< ", hasPerm=" << _router->GetPath().hasPermission()
		<< ", isDir=" << _router->GetPath().isDirectory()
		<< ", isFile=" << _router->GetPath().isFile()
		<< ", isCGI=" << _router->GetPath().isCGI();

	if (!_isMethodAllowed(method))
	{
		if (method == "GET" || method == "POST" || method == "DELETE" || method == "HEAD")
			HandleErrorPages("405");
		else
			HandleErrorPages("501");
	}
	else if (_router->GetPath().isRedirection() || _router->GetPath().isRedirectionToDir())
		_createRedirection();
	else if (_router->GetPath().isFound() == false)
		HandleErrorPages("404");
	else if (_router->GetPath().hasPermission() == false)
		HandleErrorPages("403");
	else if (_router->GetPath().isDirectory())
	{
		int idxSrv = _router->srv->autoindex, idxLoc = -1;
		if (_router->loc)
		{
			idxLoc = _router->loc->autoindex;
		}
		if ((idxSrv <= 0 && idxLoc == -1) || idxLoc == 0)
			HandleErrorPages("403");
		else
			_createListFilesResponse();
	}
	else if (_router->GetPath().isCGI())
	{
		_status = GET::eCGIResponse;
		_handleCGI();
	}
	else if (_router->GetPath().isFile())
		_serveFile();
	if (_status == GET::eInit)
		_status = GET::eSendResponse;
}

void GET::_openFile(const string &path)
{
	_fileFd = open(path.c_str(), O_RDONLY | O_CLOEXEC);
}

void GET::_prepareFileResponse()
{
	_bodySize = Utility::getFileSize(_filename);
	_statusCode = "200";
	_createResponseHeader("");
	_status = GET::eSendResponse;
	_multiplexer->ChangeToEpollOut(_sock);
}

void GET::_serveFile()
{
	DDEBUG("GET") << "Socket fd: " << _sock->GetFd() << ", ServeFile: '" << _filename << "'";
	_openFile(_filename);
	_prepareFileResponse();
	if ("HEAD" == _router->GetRequest().getMethod())
		_router->SetSendStrategy(new BuffersStrategy(_buffers, *_sock));
	else
		_router->SetSendStrategy(new FileStrategy(_buffers, _fileFd, _bodySize, *_sock));
}


string GET::_formatDirectoryEntry(const string &name, const struct stat &st, const string &requestPath)
{
	stringstream entry;

	entry << "{ name: '" << _escapeForJS(name)
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

void GET::_listFiles(const string &path)
{
	DIR *dir = opendir(path.c_str());
	if (!dir)
		return;

	struct dirent *entry;
	string requestPath = _router->GetRequest().getPath();

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
			_filesList << _formatDirectoryEntry(entryName, st, requestPath);
	}
	DDEBUG("GET") << "Socket fd: " << _sock->GetFd() << ", ListFiles: scanned directory '" << path << "'";
	closedir(dir);
}

int GET::_calculateAutoIndexSize()
{
	int size = 0;
	size += _filesList.str().size();
	size += _router->GetRequest().getPath().length();
	size += StaticFile::GetFileByName("autoIndex1")->GetSize();
	size += StaticFile::GetFileByName("autoIndex2")->GetSize();
	size += StaticFile::GetFileByName("autoIndex3")->GetSize();
	return size;
}

void GET::_createListFilesResponse()
{
	DDEBUG("GET") << "Socket fd: " << _sock->GetFd() << ", CreateListFilesResponse for path: '" << _router->GetPath().getFullPath() << "'";
	_listFiles(_router->GetPath().getFullPath());
	_filesListStr = _filesList.str();

	_statusCode = "200";
	_filename = ".html";

	_bodySize = _calculateAutoIndexSize();
	_createResponseHeader("");

	_multiplexer->ChangeToEpollOut(_sock);

	if ("HEAD" != _router->GetRequest().getMethod()) {
		_addPair(StaticFile::GetFileByName("autoIndex1"));
		_addPair(_filesListStr);
		_addPair(StaticFile::GetFileByName("autoIndex2"));
		_addPair(_router->GetRequest().getPath());
		_addPair(StaticFile::GetFileByName("autoIndex3"));
	}
	_router->SetSendStrategy(new BuffersStrategy(_buffers, *_sock));
}
