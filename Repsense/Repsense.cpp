#include "Repsense.hpp"

Repsense::Repsense()
{
	readyToSend = false;
	sendHeader = false;
	staticFile = NULL;
	servers = NULL;
	ShouldSend = 0;
	filesize = 0;
	del = false;
	sock = NULL;
	sended = 0;
	fileFd = -1;
}

void Repsense::Init(SocketIO *sock, vector<AST<string> > *servers)
{
	if (this->sock == NULL) {
		this->sock = sock;
	}
	if (this->servers == NULL) {
		this->servers = servers;
	}
}

bool Repsense::HandleResponse()
{
	Request &req = sock->GetRouter().GetRequest();
	sock->SetStateByFd(sock->GetFd());
	Logging::Debug() << "Socket fd: " << sock->GetFd() << " try to Handle Response";
	if (req.getMethod() == "GET") {
		GetMethod();
	}
	else {
		return false;
	}
	return del;
}

void Repsense::SendGetResponse()
{
	const char *h;
	int size = 0, len;
	if (staticFile != NULL) {
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
	else if (sended >= (int)responseHeaderStr.length()) {
		size = sock->FileToSocket(fileFd, ShouldSend - sended);
	}
	if (size > 0) {
		sended += size;
	}
	Logging::Debug() << "Socket fd: " << sock->GetFd() <<  
			" Send " << sended << " byte from " << ShouldSend;
	if (ShouldSend <= sended || Utility::SigPipe) {
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
	
	string serverName = Parsing::GetServerName(*(sock->GetRouter().srv));
	if (serverName == "")
		serverName = Socket::getLocalName(sock->GetFd());
	string type = Singleton::GetMime()[Utility::GetFileExtension(filename)];
	
	responseHeader 
	<< "HTTP/1.1 " << code << "\r\n"
	<< "Date: " << CreateDate() << "\r\n"
	<< "Server: " << serverName << "\r\n"
	<< "Content-Type: " << type << "\r\n"
	<< "Content-Length: " << filesize << "\r\n"
	<< "Connection: close\r\n" 
	<< "X-Content-Type-Options: nosniff\r\n"
	<< "\r\n";

	responseHeaderStr = responseHeader.str();
} 

void Repsense::HandelErrorPages(const string& err)
{
	fileFd = open(err.c_str(), O_RDONLY);
	if (fileFd == -1)
	{
		staticFile = StaticFile::GetFileByName(this->code.c_str());
		filesize = staticFile->GetSize();
		ShouldSend = filesize;
		filename = ".html";
	}
	else
	{
		filesize = Utility::getFileSize(filename);
		ShouldSend = filesize;
	}
	CreateResponseHeader();
	ShouldSend += responseHeaderStr.length();
	readyToSend = true;
	SendGetResponse();
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

void Repsense::GetStaticIndex() {
	code = "200 OK";
	staticFile = StaticFile::GetFileByName("index");
	filesize = staticFile->GetSize();
	ShouldSend = filesize;
	filename = ".html";
	CreateResponseHeader();
	ShouldSend += responseHeaderStr.length();
	readyToSend = true;
	SendGetResponse();
}

void Repsense::listFiles(string path)
{
	DIR *dir = opendir(path.c_str());
	struct dirent *entry;
	
	while ((entry = readdir(dir)) != NULL)
	{
		filesList << "{ name: '" << entry->d_name << "', href: '" << entry->d_name;
		
		struct stat st;
		if (stat(entry->d_name, &st) == 0)
		{
			time_t modTime = st.st_mtime;
			filesList << "', isDir: ";
			if (S_ISDIR(st.st_mode)) {
				filesList << "true, size: '-', date: '";
			}
			else {
				filesList << "true, size: '" << st.st_size << "', date: '" ;
			}
			filesList << ctime(&modTime) << "' },";
		}
	}
	closedir(dir);
	filesList.str();
}

void Repsense::CreateListFilesRepsense()
{

}

void Repsense::GetMethod()
{
	if (readyToSend) {
		Logging::Debug() << "Socket fd: " << sock->GetFd() << 
		" Send Get Response";
		SendGetResponse();
		return;
	}

	Routing &r = sock->GetRouter();
	try {
		filename = r.CreatePath(servers);
	} catch (exception &e) {
		if (r.GetPath().emptyRoot() && r.GetRequest().getPath() == "/"){
			GetStaticIndex();
		}
		else {
			code = r.GetPath().getErrorCode();
			HandelErrorPages(e.what());
		}
		return ;
	}
	if (r.GetPath().IsDir()) {
		
	}

	int type = checkPathType(filename);

	Logging::Debug() << "Socket fd: " << sock->GetFd() << 
	" try to send file " << filename << " with type " << getTypeName(type);
	if (type == enPathFile)
	{
		filesize = Utility::getFileSize(filename);
		ShouldSend = filesize;
		fileFd = open(filename.c_str(), O_RDONLY);
		code = "200 OK";
		CreateResponseHeader();
		ShouldSend += responseHeaderStr.length();
		readyToSend = true;
		Logging::Debug() << "Socket fd: " << sock->GetFd() << " ready To Send Response";
		SendGetResponse();
		return ;
	}
	else {
		del = true;
	}
}

Repsense::~Repsense() {
	if (fileFd != -1) {
		close(fileFd);
	}
}