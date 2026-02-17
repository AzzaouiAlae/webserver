#include "Post.hpp"

Post::Post(SocketIO *sock, Routing *router): AMethod(sock, router)
{
	fileFd = -1;
	contentBodySize = 0;
	uploadedSize = 0;
}


bool Post::HandleResponse()
{
	Request &req = router->GetRequest();
	sock->SetStateByFd(sock->GetFd());
	(void)req;

	Logging::Debug() << "Socket fd: " << sock->GetFd() << " try to Handle POST Response";

	// Check allowed methods (shared logic from AMethod)
	if (!readyToSend && !IsMethodAllowed("POST")) {
		HandelErrorPages("405");
	}

	filename = router->CreatePath(router->srv);

	if (readyToSend) {
		SendResponse();
	}
	if (readyToUpload) {
		uploadFileToDisk();
	} 
	else if (router->GetPath().isFound()) {
		HandelErrorPages("409");
	}
	else if (router->GetPath().isCGI()) {
		
	}
	else {
		PostMethod();
	}
	
	return del;
}

void Post::PostMethod()
{
	int idx = Config::GetLocationIndex(*(router->srv), router->GetRequest().getPath());
	if (idx == -1) 
	{
		HandelErrorPages("403");
		return;
	}
	fileFd = open(filename.c_str(), O_CREAT | O_WRONLY, 0666);
	if (fileFd == -1) 
	{
		HandelErrorPages("403");
		return;
	}
	contentBodySize = router->GetRequest().getContentLen();
	readyToUpload = true;
	uploadFileToDisk();
}

void  Post::uploadFileToDisk()
{
	string &s = router->GetRequest().getBody();
	int size;
	if (s.length() > uploadedSize) {
		char *h = &(s[uploadedSize]);
		size = write(fileFd, h, s.length() - uploadedSize);
	}
	else {
		size = sock->SocketToFile(fileFd, contentBodySize - uploadedSize);
	}
	if(size <= 0) 
	{
		HandelErrorPages("400");
		unlink(filename.c_str());
		return;
	}
	uploadedSize += size;
	if (uploadedSize >= contentBodySize) {
		createPostRespence();
	}
}

Post::~Post()
{
	if (fileFd != -1)
		close(fileFd);
}

void Post::createPostRespence()
{
	
}