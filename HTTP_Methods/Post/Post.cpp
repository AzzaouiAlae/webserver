#include "Post.hpp"
#include "MultipartUploadStrategy.hpp"
#include "BuffersStrategy.hpp"
#include "UploadStrategy.hpp"

Post::Post(ClientSocket *sock, Routing *router) : AMethod(sock, router)
{
	_path = &router->GetPath();
	_originalPath = &_path->OriginalPath;
	_multiplexer->ChangeToEpollIn(sock);
	DEBUG("Post")
		<< "Post initialized, socket fd="
		<< sock->GetFd();
}

Post::~Post()
{
}

void Post::_initPost()
{
	_resolvePath();

	if (!_isMethodAllowed("POST"))
		HandleErrorPages("405");
	else if (_router->GetPath().isRedirection())
		_createRedirection();
	else if (_router->GetPath().isCGI()) {
		_status = Post::eCGIResponse;
		_handleCGI();
	}
	else if (_originalPath->found && _originalPath->isFile)
		HandleErrorPages("405");
	else if (_router->loc == NULL)
		HandleErrorPages("403");
	else if (_router->GetRequest().isMultipartFormData())
		_router->SetReadStrategy(new MultipartUploadStrategy(_sock, _router));
	else
		_router->SetReadStrategy(new UploadStrategy(_router, _sock));
	if (_router->GetReadStrategy())
	{
		_status = Post::eUploadFile;
		_handleStrategyStatus(_router->GetReadStrategy());
		if (_router->GetReadStrategy()->GetStatus() == AStrategy::eComplete)
			_createPostResponse();
	}
}

bool Post::HandleResponse()
{
	DEBUG("Post")
		<< "Socket fd: " << _sock->GetFd()
		<< ", Post::HandleResponse() start";
	DDEBUG("Post") << "Socket fd: " << _sock->GetFd()
				   << ", status=" << _status;

	if (_sock->IsTimeOut())
		_createTimeoutResponse();
	else if (_status == Post::eUploadFile)
	{
		_executeStrategy(_router->GetReadStrategy());
		if (_router->GetReadStrategy()->GetStatus() == AStrategy::eComplete)
			_createPostResponse();
	}
	else if (_status == Post::eSendResponse)
	{
		_executeStrategy(_router->GetSendStrategy());
		if (_router->GetSendStrategy()->GetStatus() == AStrategy::eComplete)
			_status = eComplete;
	}
	else if (_status == Post::eInit)
		_initPost();
	else if (_status == Post::eCGIResponse)
		_handleCGI();
	return _status == Post::eComplete;
}

bool Post::_getLocationReturn(string &retCode, string &retBody)
{
	const Config::Server::Location *loc = _router->GetPath().getLocation();
	if (loc == NULL || loc->returnCode.empty())
		return false;

	retCode = loc->returnCode;
	retBody = loc->returnArg;
	return true;
}

void Post::_createPostRedirection(const string &retCode, const string &retBody)
{
	_createRedirectionHeader(retCode, retBody);
	DEBUG("Post")
		<< "Socket fd: " << _sock->GetFd()
		<< " POST redirect " << retCode
		<< " to " << retBody;
	_status = Post::eSendResponse;
	_multiplexer->ChangeToEpollOut(_sock);
}

void Post::_createPostCustomBody(const string &retCode, const string &retBody)
{
	DDEBUG("Post")
		<< "Socket fd: " << _sock->GetFd()
		<< ", CreatePostCustomBody: code="
		<< retCode << ", bodyLen="
		<< retBody.length();
	_statusCode = retCode;
	_bodySize = retBody.length();
	_filename = ".json";
	_createResponseHeader(retBody);
	_status = Post::eSendResponse;
	_multiplexer->ChangeToEpollOut(_sock);
}

void Post::_createPostResponse()
{
	string retCode, retBody;

	_multiplexer->ChangeToEpollOut(_sock);

	if (_getLocationReturn(retCode, retBody))
	{
		DDEBUG("Post")
			<< "Socket fd: " << _sock->GetFd()
			<< ", createPostResponse: location return code="
			<< retCode;

		if (retCode == "301" || retCode == "302")
			_createPostRedirection(retCode, retBody);
		else
			_createPostCustomBody(retCode, retBody);
	}
	else
	{
		DDEBUG("Post")
			<< "Socket fd: " << _sock->GetFd()
			<< ", createPostResponse: no return directive, sending 201 Created.";
		_createDefaultResponse("201");
	}
	_router->SetSendStrategy(new BuffersStrategy(_buffers, *_sock));
}
