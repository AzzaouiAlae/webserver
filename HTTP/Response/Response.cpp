#include "Response.hpp"

Repsense::Repsense()
{
	_method = NULL;
	_sock = NULL;
	_router = NULL;
	_status = Repsense::eCreateMethod;
}

Repsense::~Repsense()
{
	if (_method != NULL)
	{
		delete _method;
		_method = NULL;
	}
}

void Repsense::Init(SocketIO *sock, Routing *router)
{
	this->_sock = sock;
	this->_router = router;
}

void Repsense::_createMethod()
{
	string requestMethod = _router->GetRequest().getMethod();

	if (requestMethod == "GET") {
		DEBUG("Repsense") << "Socket fd: " << _sock->GetFd() << ", creating GET handler";
		_method = new GET(_sock, _router);
	}
	else if (requestMethod == "POST") {
		DEBUG("Repsense") << "Socket fd: " << _sock->GetFd() << ", creating POST handler";
		_method = new Post(_sock, _router);
	}
	else if (requestMethod == "DELETE") {
		DEBUG("Repsense") << "Socket fd: " << _sock->GetFd() << ", creating DELETE handler";
		_method = new Delete(_sock, _router);
	}
	else
		_method = new GET(_sock, _router);
	_status = Repsense::eHandleResponse;
}

bool Repsense::HandleResponse()
{
	if (_status == Repsense::eCreateMethod)
		_createMethod();
	if (_status == Repsense::eHandleResponse && _method != NULL)
		return _method->HandleResponse();
	return false;
}

void Repsense::HandelErrorPages(const string &err)
{
	if (_method == NULL)
	{
		_method = new GET(_sock, _router);
	}
	_method->HandelErrorPages(err);
	_status = Repsense::eHandleResponse;
}
