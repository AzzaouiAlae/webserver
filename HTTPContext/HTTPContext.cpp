#include "HTTPContext.hpp"

void HTTPContext::Handle(AFd *fd)
{
	if (fd->GetType() == "Socket")
		Handle((Socket *)fd);
	else if (fd->GetType() == "SocketIO")
		Handle();
}

void HTTPContext::Handle(Socket *sock)
{
	SocketIO *fd = new SocketIO(sock->acceptedSocket);
	Singleton::GetFds().insert(fd);
	servers = &((Singleton::GetServers())[sock->GetFd()]);
	fd->context = new HTTPContext();
	((HTTPContext *)fd->context)->servers = servers;
	((HTTPContext *)fd->context)->sock = fd;
	Multiplexer::GetCurrentMultiplexer()->AddAsEpollIn(fd);
	Logging::Debug() << "Socket FD: " << sock->GetFd() 
	<< " accepte new connection as socket fd: " << sock->acceptedSocket;
}

void HTTPContext::Handle()
{
	Routing &r = sock->GetRouter();

	if (r.isRequestComplete() == false) {
		Logging::Debug()  << "Socket FD: " << sock->GetFd() << 
		" start handle request";
		HandleRequest();
	}
	else {
		Logging::Debug()  << "Socket FD: " << sock->GetFd() << 
		" start handle response";
		HandleResponse();
	}
}

void HTTPContext::HandleRequest()
{
	int len = 0;
	Routing &r = sock->GetRouter();
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();
	if (buf == NULL) {
		buf = (char *)calloc(1, BUF_SIZE + 1);
	}
	len = read(sock->GetFd(), buf, BUF_SIZE);
	Logging::Debug() << "Socket FD: " << sock->GetFd() << 
		" read " << len << " byte";
	if (len == 0 || Utility::SigPipe) {
		MarkedSocketToFree();
	}
	if (len != -1)
	{
		buf[len] = '\0';
		if (r.GetRequest().isComplete(buf))
		{
			Logging::Debug() << "Read of Request from Socket fd: " << sock->GetFd() << " Complete";
			Logging::Debug() << "Request is : " << r.GetRequest().getMethod() <<  " " << r.GetRequest().getPath();
			r.SetRequestComplete();
			MulObj->ChangeToEpollOut(sock);

			in = new Pipe(sock->pipefd[0], sock);
			MulObj->AddAsEpollIn(in);

			out = new Pipe(sock->pipefd[1], sock);
			MulObj->AddAsEpollOut(out);
		}
	}
}

void HTTPContext::HandleResponse()
{
	Request &req = sock->GetRouter().GetRequest();
	sock->SetStateByFd(sock->GetFd());
	Logging::Debug() << "Socket fd: " << sock->GetFd() << " try to Handle Response";
	if (req.getMethod() == "GET") {
		GetMethod();
	}
	else {
		MarkedSocketToFree();
	}
}

enum enPathType
{
	enPathError,
	enPathFolder,
	enPathFile,
	enPathOther
};

int checkPathType(const std::string &path)
{
	struct stat info;

	// stat returns 0 on success
	if (stat(path.c_str(), &info) != 0)
	{
		// std::cout << "Path does not exist or cannot be accessed." << std::endl;
		return enPathError;
	}

	// Checking the st_mode bitmask
	if (info.st_mode & S_IFDIR)
	{
		return enPathFolder;
	}
	else if (info.st_mode & S_IFREG)
	{
		return enPathFile;
	}
	else
	{
		return enPathOther;
	}
}

string getTypeName(int type)
{
	switch (type)
	{
	case enPathError: return "enPathError";
	case enPathFolder: return "enPathFolder";
	case enPathFile: return "enPathFile";
	case enPathOther: return "enPathOther";
	}
	return "";
}

HTTPContext::HTTPContext()
{
	sended = 0;
	readyToSend = false;
	in = NULL;
	out = NULL;
	sock = NULL;
	fileFd = -1;
	buf = NULL;
}

void HTTPContext::GetMethod()
{
	if (readyToSend) {
		Logging::Debug() << "Socket fd: " << sock->GetFd() << 
		" Send Get Response";
		SendGetResponse();
		return;
	}
	
	Routing &r = sock->GetRouter();
	filename = r.CreatePath(servers);
	int type = checkPathType(filename);

	Logging::Debug() << "Socket fd: " << sock->GetFd() << 
	" try to send file " << filename << " with type " << getTypeName(type);
	if (type == enPathFile)
	{
		filesize = Utility::getFileSize(filename);
		ShouldSend = filesize;
		fileFd = open(filename.c_str(), O_RDONLY);
		if (fileFd == -1)
		{
			MarkedSocketToFree();
			return;
		}
		CreateResponseHeader();
		ShouldSend += responseHeaderStr.length();
		readyToSend = true;
		Logging::Debug() << "Socket fd: " << sock->GetFd() << " ready To Send Response";
		SendGetResponse();
		return ;
	}
	else
	{
		MarkedSocketToFree();
	}
}

void HTTPContext::MarkedSocketToFree()
{
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();

	Utility::SigPipe = false;
	if (in != NULL)
		MulObj->ChangeToEpollOneShot(in);
	if (in != NULL)
		MulObj->ChangeToEpollOneShot(out);
	shutdown(sock->GetFd(), SHUT_RDWR);
	sock->MarkedToFree = true;
}

void HTTPContext::SendGetResponse()
{
	int size = 0;
	
	if (sended < (int)responseHeaderStr.length())
	{
		const char *h = &((responseHeaderStr.c_str())[sended]);
		int len = responseHeaderStr.length() - sended;
		size = sock->sendFileWithHeader(h, len, fileFd, ShouldSend - sended);
		if (size > 0) {
			sended += size;
		}
	}
	else if (sended >= (int)responseHeaderStr.length()) {
		size = sock->FileToSocket(fileFd, ShouldSend - sended);
		if (size > 0) {
			sended += size;
		}
	}
	Logging::Debug() << "Socket fd: " << sock->GetFd() <<  
			" Send " << sended << " byte from " << ShouldSend;
	if (ShouldSend == sended || Utility::SigPipe) {
		MarkedSocketToFree();
	}
}

string HTTPContext::GetStatusCode()
{
	return "200 OK";
}

string HTTPContext::CreateDate()
{
	char date[100];
	time_t now = time(0);
	struct tm tm = *gmtime(&now);
	strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S GMT", &tm);
	return date;
}

void HTTPContext::CreateResponseHeader()
{
	responseHeader.str("");
	
	string serverName = Parsing::GetServerName(*(sock->GetRouter().srv));
	if (serverName == "")
		serverName = Socket::getLocalName(sock->GetFd());
	string type = Singleton::GetMime()[Utility::GetFileExtension(filename)];
	
	responseHeader 
	<< "HTTP/1.1 " << GetStatusCode() << "\r\n"
	<< "Date: " << CreateDate() << "\r\n"
	<< "Server: " << serverName << "\r\n"
	<< "Content-Type: " << type << "\r\n"
	<< "Content-Length: " << filesize << "\r\n"
	<< "Connection: close\r\n" 
	<< "X-Content-Type-Options: nosniff\r\n"
	<< "\r\n";

	responseHeaderStr = responseHeader.str();
} 

void CreateNotFoundError()
{}

void ListFiles()
{}

HTTPContext::~HTTPContext()
{
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();

	if (in) {
		MulObj->DeleteFromEpoll(in);
		delete in;
	}
	if (out) {
		MulObj->DeleteFromEpoll(out);
		delete out;
	}
	if (fileFd != -1) {
		close(fileFd);
	}
}

void HTTPContext::activeInPipe() {
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();

	MulObj->ChangeToEpollIn(in);
}

void HTTPContext::activeOutPipe() {
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();

	MulObj->ChangeToEpollIn(out);
}