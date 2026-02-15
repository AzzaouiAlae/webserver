#include "Repsense.hpp"

Repsense::Repsense()
{
	readyToSend = false;
	sendHeader = false;
	staticFile = NULL;
	servers = NULL;
	ShouldSend = 0;
	bodySize = 0;
	del = false;
	sock = NULL;
	sended = 0;
	fileFd = -1;
	sendListFiles = 0;
	sent = -1;
	targetToSend = 0;
	router = 0;
}

void Repsense::Init(SocketIO *sock, vector<AST<string> > *servers, Routing *router)
{
	if (this->sock == NULL)
	{
		this->sock = sock;
	}
	if (this->servers == NULL)
	{
		this->servers = servers;
	}
	if (this->router == NULL)
	{
		this->router = router;
	}
}

bool Repsense::HandleResponse()
{
	Request &req = router->GetRequest();
	sock->SetStateByFd(sock->GetFd());
	Logging::Debug() << "Socket fd: " << sock->GetFd() << " try to Handle Response";
	if (req.getMethod() == "GET")
	{
		GetMethod();
	}
	else
	{
		return true;
	}
	return del;
}

void Repsense::SendGetResponse()
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
	{
		sended += size;
	}
	Logging::Debug() << "Socket fd: " << sock->GetFd() << " Send " << sended << " byte from " << ShouldSend;
	if (ShouldSend <= sended || Utility::SigPipe)
	{
		del = true;
	}
}

string Repsense::CreateDate()
{
	char date[100];
	time_t now = time(0);
	struct tm tm = *gmtime(&now);
	strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S GMT", &tm);
	return date;
}

void Repsense::CreateResponseHeader()
{
	responseHeader.str("");

	string serverName = Parsing::GetServerName(*(router->srv));
	if (serverName == "")
		serverName = Socket::getLocalName(sock->GetFd());
	string type = Singleton::GetMime()[Utility::GetFileExtension(filename)];

	responseHeader
		<< "HTTP/1.1 " << code << "\r\n"
		<< "Date: " << CreateDate() << "\r\n"
		<< "Server: " << serverName << "\r\n"
		<< "Content-Type: " << type << "\r\n"
		<< "Content-Length: " << bodySize << "\r\n"
		<< "Connection: close\r\n"
		<< "X-Content-Type-Options: nosniff\r\n"
		<< "\r\n";

	responseHeaderStr = responseHeader.str();
}

void Repsense::HandelErrorPages(const string &err)
{
	fileFd = open(err.c_str(), O_RDONLY);
	if (fileFd == -1)
	{
		staticFile = StaticFile::GetFileByName(code.c_str());
		bodySize = staticFile->GetSize();
		ShouldSend = bodySize;
		filename = ".html";
	}
	else
	{
		bodySize = Utility::getFileSize(filename);
		ShouldSend = bodySize;
	}
	CreateResponseHeader();
	ShouldSend += responseHeaderStr.length();
	readyToSend = true;
	SendGetResponse();
}

void Repsense::GetStaticIndex()
{
	code = "200 OK";
	staticFile = StaticFile::GetFileByName("index");
	bodySize = staticFile->GetSize();
	ShouldSend = bodySize;
	filename = ".html";
	CreateResponseHeader();
	ShouldSend += responseHeaderStr.length();
	readyToSend = true;
	SendGetResponse();
}

void Repsense::listFiles(std::string path)
{
	DIR *dir = opendir(path.c_str());
	if (!dir)
		return;

	struct dirent *entry;
	std::string rp = router->GetRequest().getPath();

	while ((entry = readdir(dir)) != NULL)
	{
		if (string(entry->d_name) == "." ||
			(string(entry->d_name) == ".." && rp == "/") )
			continue;

		std::string fullPath = path;
		if (fullPath[fullPath.size() - 1] != '/')
			fullPath += "/";
		fullPath += entry->d_name;

		struct stat st;
		if (stat(fullPath.c_str(), &st) == 0)
		{
			filesList 	<< "{ name: '" << entry->d_name
						<< "', href: '" << rp
						<< (rp[rp.size() - 1] == '/' ? "" : "/")
						<< entry->d_name
						<< "', isDir: ";

			if (S_ISDIR(st.st_mode))
			{
				filesList << "true, size: '-', date: '";
			}
			else
			{
				filesList 	<< "false, size: '"
							<< st.st_size
							<< "', date: '";
			}

			time_t modTime = st.st_mtime;
			std::string s = ctime(&modTime);
			filesList << s.substr(0, s.find("\n")) << "' },\n";
		}
	}

	closedir(dir);
}

void Repsense::CreateListFilesRepsense()
{
	listFiles(router->GetPath().getFullPath());
	filesListStr = filesList.str();
	ShouldSend = filesList.str().size();

	ShouldSend += router->GetRequest().getPath().length();

	ShouldSend += StaticFile::GetFileByName("autoIndex1")->GetSize();
	ShouldSend += StaticFile::GetFileByName("autoIndex2")->GetSize();
	ShouldSend += StaticFile::GetFileByName("autoIndex3")->GetSize();

	code = "200 OK";
	filename = ".html";

	bodySize = ShouldSend;
	CreateResponseHeader();
	ShouldSend += responseHeaderStr.length();

	sendListFiles = 1;
	SendListFilesRepsense();
}

void Repsense::SendListFilesStr(const string &str)
{
	int len = targetToSend - sended;
	int s = str.length() - len;
	Logging::Debug() << "SendListFilesStr s: " << s << ", len: " << len << ", targetToSend: "
					 << targetToSend << ", sended: " << sended << ", str: \n"
					 << str << "str len: " << str.length();
	const char *b = &(str[s]);
	Logging::Debug() << "b addr: " << &b << ", b[0] addr: " << (void *)(&(b[0]));
	int sent = sock->Send((void *)b, len);
	if (sent > 0)
	{
		sended += sent;
	}

	Logging::Debug() << "SendListFilesStr sent " << sent << " bytes";
}

void Repsense::SendAutoIndex(StaticFile *f)
{
	int len = targetToSend - sended;
	int s = f->GetSize() - len;
	const char *b = &((f->GetData())[s]);
	int sent = sock->Send((void *)b, len);
	if (sent > 0)
	{
		sended += sent;
	}
	Logging::Debug() << "SendAutoIndex sent " << sent << " bytes";
}

void Repsense::SendListFilesRepsense()
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
	if (sended >= targetToSend)
		sendListFiles++;
	if (sendListFiles == 7 || Utility::SigPipe)
		del = true;
}

void Repsense::ServeFile()
{
	bodySize = Utility::getFileSize(filename);
	ShouldSend = bodySize;
	fileFd = open(filename.c_str(), O_RDONLY);
	code = "200 OK";
	CreateResponseHeader();
	ShouldSend += responseHeaderStr.length();
	readyToSend = true;
	Logging::Debug() << "Socket fd: " << sock->GetFd() << " ready To Send Response";
	SendGetResponse();
}

void Repsense::GetMethod()
{
	if (sendListFiles)
	{
		SendListFilesRepsense();
		return;
	}
	if (readyToSend)
	{
		Logging::Debug() << "Socket fd: " << sock->GetFd() << " Send Get Response";
		SendGetResponse();
		return;
	}

	try {
		filename = router->CreatePath(servers);
	}
	catch (exception &e) {
		if (router->GetPath().emptyRoot() && router->GetRequest().getPath() == "/")
			GetStaticIndex();
		else
		{
			code = router->GetPath().getErrorCode();
			HandelErrorPages(e.what());
		}
		return;
	}

	Logging::Debug() << "Socket fd: " << sock->GetFd() << 
	" try to send file " << filename;

	if (router->GetPath().IsDir())
		CreateListFilesRepsense();
	else
		ServeFile();
}

Repsense::~Repsense()
{
	if (fileFd != -1)
	{
		close(fileFd);
	}
}