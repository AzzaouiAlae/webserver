#include "Delete.hpp"

Delete::Delete(SocketIO *sock, Routing *router) : AMethod(sock, router)
{
	DEBUG("Delete") << "Delete initialized, socket fd=" << sock->GetFd();
}

void Delete::_initDelete()
{
	_resolvePath();
	_loc = _router->GetPath().getLocation();

	DDEBUG("Delete") 
		<< "Socket fd: " << _sock->GetFd()
		<< ", found=" << _router->GetPath().isFound()
		<< ", hasPerm=" << _router->GetPath().hasPermission()
		<< ", isCGI=" << _router->GetPath().isCGI();

	if (!_isMethodAllowed("DELETE"))
		HandelErrorPages("405");
	else if (_router->GetPath().isFound() == false)
		HandelErrorPages("404");
	else if (_router->GetPath().hasPermission() == false)
		HandelErrorPages("403");
	else if (_router->GetPath().isCGI())
		_handelCGI();
	else if (_loc == NULL || _loc->deleteFiles == false)
		HandelErrorPages("403");
	else 
		_deleteFile();
}

bool Delete::HandleResponse()
{
	_sock->SetStateByFd(_sock->GetFd());

	DEBUG("Delete") 
		<< "Socket fd: " << _sock->GetFd() 
		<< ", Delete::HandleResponse() start";

	if (_sock->isTimeOut())
		_createTimeoutResponse();
	else if (_status == Delete::eInit)
		_initDelete();
	else if (_status == Delete::eSendResponse)
		_sendResponse();
	else if (_status == Delete::eCGIResponse)
		_handelCGI();

	return _status == Delete::eComplete;
}

void Delete::_deleteFile()
{
	DDEBUG("Delete") << "Socket fd: " << _sock->GetFd() << ", deleteFile: '" << _filename << "'";
	if (unlink(_filename.c_str()))
	{
		ERR() << "Client " << Socket::getRemoteName(_sock->GetFd()) << " delete failed: " << _filename;
		DDEBUG("Delete") << "Socket fd: " << _sock->GetFd() << ", unlink failed, sending 500.";
		HandelErrorPages("500");
	}
	else
	{
		INFO() << "Client " << Socket::getRemoteName(_sock->GetFd()) << " file deleted: " << _filename;
		DDEBUG("Delete") << "Socket fd: " << _sock->GetFd() << ", file deleted successfully.";
		_sendDefaultRespense("204");
	}
}