#include "Post.hpp"

Post::Post(SocketIO *sock, Routing *router): AMethod(sock, router)
{

}


bool Post::HandleResponse()
{
	Request &req = router->GetRequest();
	sock->SetStateByFd(sock->GetFd());

	Logging::Debug() << "Socket fd: " << sock->GetFd() << " try to Handle POST Response";

	// Check allowed methods (shared logic from AMethod)
	if (!readyToSend && !IsMethodAllowed("POST"))
		HandelErrorPages("405");
	
	
	
	return true;
}