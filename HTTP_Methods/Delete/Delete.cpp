#include "Delete.hpp"

Delete::Delete(SocketIO *sock, Routing *router) : AMethod(sock, router)
{
	DEBUG("Delete") << "Delete initialized, socket fd=" << sock->GetFd();
}

bool Delete::HandleResponse()
{
	sock->SetStateByFd(sock->GetFd());

	DEBUG("Delete") << "Socket fd: " << sock->GetFd() << ", Delete::HandleResponse() start";

	// 1. Already sending response (success or error) → keep sending
	if (readyToSend)
	{
		SendResponse();
		return del;
	}

	// 2. Check method is allowed
	if (!IsMethodAllowed("DELETE"))
	{
		DDEBUG("Delete") << "Socket fd: " << sock->GetFd() << ", DELETE method not allowed, sending 405.";
		HandelErrorPages("405");
		return del;
	}

	ResolvePath();

	DDEBUG("Delete") << "Socket fd: " << sock->GetFd()
					  << ", found=" << router->GetPath().isFound()
					  << ", hasPerm=" << router->GetPath().hasPermission()
					  << ", isCGI=" << router->GetPath().isCGI()
					  << ", emptyRoot=" << router->GetPath().emptyRoot();

	if (router->GetPath().isFound() == false) {
		HandelErrorPages("404");
		return del;
	}

	if (router->GetPath().hasPermission() == false) {
		HandelErrorPages("403");
		return del;
	}

	if (router->GetPath().isCGI()) {
		DDEBUG("Delete") << "Socket fd: " << sock->GetFd() << ", CGI path detected (not yet implemented).";
		//
		return del;
	}

	loc = router->GetPath().getLocation();

	if (loc == NULL || router->GetPath().emptyRoot() || loc->deleteFiles == false) {
		DDEBUG("Delete") << "Socket fd: " << sock->GetFd() << ", delete not permitted (loc="
						  << (loc != NULL ? "set" : "NULL")
						  << ", emptyRoot=" << router->GetPath().emptyRoot()
						  << ", deleteFiles=" << (loc != NULL ? (loc->deleteFiles ? "true" : "false") : "N/A") << ").";
		HandelErrorPages("403");
		return del;
	}

	deleteFile();

	return del;
}

void Delete::deleteFile()
{
	DDEBUG("Delete") << "Socket fd: " << sock->GetFd() << ", deleteFile: '" << filename << "'";
	if (unlink(filename.c_str())) {
		ERR() << "Client " << Socket::getRemoteName(sock->GetFd()) << " delete failed: " << filename;
		DDEBUG("Delete") << "Socket fd: " << sock->GetFd() << ", unlink failed, sending 500.";
		HandelErrorPages("500");
	}
	else {
		INFO() << "Client " << Socket::getRemoteName(sock->GetFd()) << " file deleted: " << filename;
		DDEBUG("Delete") << "Socket fd: " << sock->GetFd() << ", file deleted successfully.";
		if (router->GetPath().isRedirection())
			SendRedirection();
		else {
			SendDefaultRespense();
		}
	}
}