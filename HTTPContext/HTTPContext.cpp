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
		HandleRequest();
	}
	else {
		HandleResponse();
	}
}

void HTTPContext::HandleRequest()
{
	Routing &r = sock->GetRouter();
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();
	char buff[1024 * KBYTE];
	int len = read(sock->GetFd(), buff, 1024 * KBYTE -1);

	if (len != -1)
	{
		buff[len] = '\0';
		if (r.GetRequest().isComplete(buff))
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
		sock->MarkedToFree = true;
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
}

void HTTPContext::GetMethod()
{
	if (readyToSend) {
		SendResponse();
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
			shutdown(sock->GetFd(), SHUT_RDWR);
			sock->MarkedToFree = true;
			return;
		}
		CreateResponseHeader();
		ShouldSend += responseHeaderStr.length();
		readyToSend = true;
		Logging::Debug() << "Socket fd: " << sock->GetFd() << " ready To Send Response";
		SendResponse();
		return ;
	}
	else
	{
		shutdown(sock->GetFd(), SHUT_RDWR);
		sock->MarkedToFree = true;
	}
}

void HTTPContext::SendResponse()
{
	int size = 0;
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();
	if (sended < (int)responseHeaderStr.length())
	{
		const char *h = &((responseHeaderStr.c_str())[sended]);
		int len = responseHeaderStr.length() - sended;
		size = sock->sendFileWithHeader(h, len, fileFd);
		if (size == -1) {
			
		}
		else
			sended += size;
	}
	else if (sended >= (int)responseHeaderStr.length()) {
		size = sock->FileToSocket(fileFd);
		if (size > 0)
			sended += size;
	}
	Logging::Debug() << "Socket fd: " << sock->GetFd() <<  
			" Send " << sended << " byte from " << filesize + (int)responseHeaderStr.length();
	if (filesize + (int)responseHeaderStr.length() <= sended) {
		MulObj->ChangeToEpollOneShot(in);
		MulObj->ChangeToEpollOneShot(out);
		// shutdown(sock->GetFd(), SHUT_RDWR);
		sock->MarkedToFree = true;
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