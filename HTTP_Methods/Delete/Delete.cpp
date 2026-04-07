#include "Delete.hpp"
#include "BuffersStrategy.hpp"

Delete::Delete(ClientSocket *sock, Routing *router) : AMethod(sock, router)
{
	DEBUG("Delete") << "Delete initialized, socket fd=" << sock->GetFd();
}

void Delete::_initDelete()
{
	_resolvePath();

	DDEBUG("Delete") 
		<< "Socket fd: " << _sock->GetFd()
		<< ", found=" << _router->GetPath().isFound()
		<< ", hasPerm=" << _router->GetPath().hasPermission()
		<< ", isCGI=" << _router->GetPath().isCGI();

	if (!_isMethodAllowed("DELETE"))
		HandleErrorPages("405");
	else if (_router->GetPath().isFound() == false)
		HandleErrorPages("404");
	else if (_router->GetPath().hasPermission() == false)
		HandleErrorPages("403");
	else if (_router->GetPath().isCGI()) {
		_status = Delete::eCGIResponse;
		_handleCGI();
	}
	else if (_router->loc == NULL || _router->loc->deleteFiles == false)
		HandleErrorPages("403");
	else 
		_deleteFile();
}

bool Delete::HandleResponse()
{
	DEBUG("Delete") 
		<< "Socket fd: " << _sock->GetFd() 
		<< ", Delete::HandleResponse() start";

	if (_sock->IsTimeOut())
		_createTimeoutResponse();
	else if (_status == Delete::eInit)
		_initDelete();
	else if (_status == Delete::eSendResponse) {
		_executeStrategy(_router->GetSendStrategy());
		if (_router->GetSendStrategy()->GetStatus() == AStrategy::eComplete)
			_status = Delete::eComplete;
	}
	else if (_status == Delete::eCGIResponse)
		_handleCGI();

	return _status == Delete::eComplete;
}

void Delete::_deleteFile()
{
	DDEBUG("Delete") << "Socket fd: " << _sock->GetFd() << ", deleteFile: '" << _filename << "'";
	if (unlink(_filename.c_str()))
	{
		ERR() << "Client " << Socket::getRemoteName(_sock->GetFd()) << " delete failed: " << _filename;
		DDEBUG("Delete") << "Socket fd: " << _sock->GetFd() << ", unlink failed, sending 500.";
		HandleErrorPages("500");
	}
	else
	{
		INFO() << "Client " << Socket::getRemoteName(_sock->GetFd()) << " file deleted: " << _filename;
		DDEBUG("Delete") << "Socket fd: " << _sock->GetFd() << ", file deleted successfully.";
		_createDefaultResponse("204");
		_router->SetSendStrategy(new BuffersStrategy(_buffers, *_sock));
		_handleStrategyStatus(_router->GetSendStrategy());
	}
}