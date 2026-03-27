#include "GET.hpp"

GET::GET(SocketIO *sock, Routing *router): AMethod(sock, router)
{
	_sendListFiles = 0;
	_targetToSend = 0;
	_sent = -1;
	_filesListStr = "";
	DEBUG("GET") << "GET initialized, socket fd=" << sock->GetFd();
}

GET::~GET()
{}

bool GET::HandleResponse()
{
	if (_sock->isTimeOut())
		_createTimeoutResponse();
	else if (_status ==  GET::eCreateResponse)
		_createResponse();
	else if (_status == GET::eCGIResponse) {
		_handelCGI();
	}
	else if (_status == GET::eSendAutoIndex) {
		_sendListFilesResponse();
	}
	else {
		_sendResponse();
	}
	
	return _status == GET::eComplete;
}

void GET::_createResponse() {
	DEBUG("GET") << "Socket fd: " << _sock->GetFd() << ", GET::HandleResponse() start";

	ClientRequest &req = _router->GetRequest();
	_sock->SetStateByFd(_sock->GetFd());
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
			HandelErrorPages("405");
		else 
			HandelErrorPages("501");
	}
	else if (_router->GetPath().isRedirection() || _router->GetPath().isRedirectionToDir())
		_sendRedirection();
	else if (_router->GetPath().isFound() == false)
		HandelErrorPages("404");
	else if (_router->GetPath().hasPermission() == false)
		HandelErrorPages("403");
	else if (_router->GetPath().isDirectory()) {
		int idxSrv = _router->srv->autoindex, idxLoc = -1;
		if (_router->loc) {
			idxLoc = _router->loc->autoindex;
		}
		if ((idxSrv <= 0 && idxLoc == -1) || idxLoc == 0 )
			HandelErrorPages("403");
		else
			_createListFilesResponse();
	}
	else if (_router->GetPath().isCGI()) {
		_handelCGI();
	}
	else if (_router->GetPath().isFile()) {
		_serveFile();
	}
	if (_status == GET::eCreateResponse) {
		_status = GET::eSendResponse;
	}
}


void GET::_openFile(const string &path)
{
	_fileFd = open(path.c_str(), O_RDONLY | O_CLOEXEC);
}

void GET::_prepareFileResponse()
{
	_bodySize = Utility::getFileSize(_filename);
	_totalByteToSend = _bodySize;
	_statusCode = "200";
	_createResponseHeader();
	if ("HEAD" == _router->GetRequest().getMethod())
		_totalByteToSend = 0;
	_totalByteToSend += _responseHeaderStr.length();
	_status = GET::eSendResponse;
	_multiplexer->ChangeToEpollOut(_sock);
}

void GET::_serveFile()
{
	DDEBUG("GET") << "Socket fd: " << _sock->GetFd() << ", ServeFile: '" << _filename << "'";
	_openFile(_filename);
	_prepareFileResponse();
}

void GET::_getStaticIndex()
{
	_statusCode = "200";
	_staticFile = StaticFile::GetFileByName("index");
	_bodySize = _staticFile->GetSize();
	_totalByteToSend = _bodySize;
	_filename = ".html";
	_createResponseHeader();
	if ("HEAD" == _router->GetRequest().getMethod())
		_totalByteToSend = 0;
	_totalByteToSend += _responseHeaderStr.length();
	_status = GET::eSendResponse;
	_multiplexer->ChangeToEpollOut(_sock);
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

	_totalByteToSend = _calculateAutoIndexSize();

	_statusCode = "200";
	_filename = ".html";

	_bodySize = _totalByteToSend;
	_createResponseHeader();
	if ("HEAD" == _router->GetRequest().getMethod())
		_totalByteToSend = 0;
	_totalByteToSend += _responseHeaderStr.length();

	_status = eSendAutoIndex;
	_sendListFiles = 1;
	_multiplexer->ChangeToEpollOut(_sock);
}

void GET::_sendChunk(const char *data, int dataSize)
{
	int sent = _sock->Send((void *)data, dataSize);
	if (sent > 0) {
		_sended += sent;
		_sock->UpdateTime();
	}
}

void GET::_sendListFilesStr(const string &str)
{
	int len = _targetToSend - _sended;
	int offset = str.length() - len;

	const char *b = &(str[offset]);
	_sendChunk(b, len);
}

void GET::_sendAutoIndex(StaticFile *f)
{
	int len = _targetToSend - _sended;
	int offset = f->GetSize() - len;
	const char *b = &((f->GetData())[offset]);

	_sendChunk(b, len);
}

void GET::_advanceListFilesState()
{
	if (_sended >= _targetToSend)
		_sendListFiles++;
	if (_sendListFiles == 7 || Utility::SigPipe || _sock->errorNumber == eWriteError) {
		_status = eComplete;
	}
}

void GET::_sendListFilesResponse()
{
	int s1 = _responseHeaderStr.length();
	int s2 = _filesList.str().length();
	int s3 = _router->GetRequest().getPath().length();
	int l1 = StaticFile::GetFileByName("autoIndex1")->GetSize();
	int l2 = StaticFile::GetFileByName("autoIndex2")->GetSize();
	int l3 = StaticFile::GetFileByName("autoIndex3")->GetSize();

	switch (_sendListFiles)
	{
	case 1:
		_targetToSend = s1;
		_sendListFilesStr(_responseHeaderStr);
		break;
	case 2:
		_targetToSend = s1 + l1;
		_sendAutoIndex(StaticFile::GetFileByName("autoIndex1"));
		break;
	case 3:
		_targetToSend = s1 + l1 + s2;
		_sendListFilesStr(_filesListStr);
		break;
	case 4:
		_targetToSend = s1 + l1 + s2 + l2;
		_sendAutoIndex(StaticFile::GetFileByName("autoIndex2"));
		break;
	case 5:
		_targetToSend = s1 + l1 + s2 + l2 + s3;
		_sendListFilesStr(_router->GetRequest().getPath());
		break;
	case 6:
		_targetToSend = s1 + l1 + s2 + l2 + s3 + l3;
		_sendAutoIndex(StaticFile::GetFileByName("autoIndex3"));
		break;
	}
	_advanceListFilesState();
}
