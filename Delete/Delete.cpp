#include "Delete.hpp"

bool Delete::HandleResponse()
{
	sock->SetStateByFd(sock->GetFd());

	Logging::Debug() << "Socket fd: " << sock->GetFd() << " try to Handle Delete Response";

	// 1. Already sending response (success or error) → keep sending
	if (readyToSend)
	{
		SendResponse();
		return del;
	}

	// 2. Check method is allowed
	if (!IsMethodAllowed("Delete"))
	{
		HandelErrorPages("405");
		return del;
	}

	ResolvePath();

	if (router->GetPath().isFound() == false) {
		HandelErrorPages("404");
		return del;
	}

	if (router->GetPath().hasPermission() == false) {
		HandelErrorPages("403");
		return del;
	}

	if (router->GetPath().isCGI()) {
		//
		return del;
	}

	deleteFile();

	return false;
}

void Delete::deleteFile()
{
	Config::Server::Location *loc = router->GetPath().getLocation();

	if (loc == NULL)
		return;
	
}