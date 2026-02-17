#include "Post.hpp"

Post::Post(SocketIO *sock, Routing *router): AMethod(sock, router)
{

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

	PostMethod();


	
	
	
	return del;
}

void Post::PostMethod()
{
	filename = router->CreatePath(router->srv);

	if (readyToUpload) {
		
	} 
	else if (router->GetPath().isFound()) {
		HandelErrorPages("409");
	}
	else if (router->GetPath().isCGI()) {
		
	}
	else {
		int idx = Config::GetLocationIndex(*(router->srv), router->GetRequest().getPath());
		if (idx == -1) {
			
		}
	}
}

void uploadFileToDisk()
{

}