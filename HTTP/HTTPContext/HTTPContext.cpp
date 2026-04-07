#include "HTTPContext.hpp"

void HTTPContext::Handle(AFd *fd)
{
	(void)fd;
	HTTPLog(DDEBUG) << "Handle() start";
	if (_status == HTTPContext::eReadingRequest)
	{
		_handleRequest();
	}
	if (_status == HTTPContext::eWritingResponse)
	{
		if (_repsense.HandleResponse())
			_status = HTTPContext::eDone;
	}
	if (_status == eDone || _status == eDelete)
	{
		HTTPLog(DDEBUG) << "Handle() done";
		_handleConnectionEnd();
	}
}

void HTTPContext::_readFromSocket()
{
	Utility::SigPipe = false;
	_readLen = read(_sock->GetFd(), _buf, BUF_SIZE);
	_sock->UpdateTime();
	HTTPLog(DDEBUG)
		<< "_readFromSocket: read returned "
		<< _readLen;
	if (_readLen <= 0 || Utility::SigPipe)
	{
		HTTPLog(DDEBUG) << "_readFromSocket: connection closed or SigPipe, setting err=true.";
		_status = HTTPContext::eDelete;
	}
}

void HTTPContext::_setMaxBodySize()
{
	_router.srv = &Config::GetServerName(*_servers, _router.GetRequest().getHost());
	_router.CreatePath(_router.srv);

	if (_router.loc != NULL && _router.loc->isMaxBodySize)
	{
		_router.GetRequest().SetMaxBodySize(_router.loc->clientMaxBodySize);
		HTTPLog(DDEBUG)
			<< ", max body size set from location: "
			<< _router.GetPath().getLocation()->clientMaxBodySize;
	}
	else if (_router.srv->isMaxBodySize)
	{
		_router.GetRequest().SetMaxBodySize(_router.srv->clientMaxBodySize);
		HTTPLog(DDEBUG)
			<< ", max body size set from server: "
			<< _router.srv->clientMaxBodySize;
	}
	_isMaxBodyInit = true;
	if (_router.GetPath().isCGI())
		_sock->SetTimeout(_router.srv->cgiTimeout);
	else
		_sock->SetTimeout(_router.srv->clientReadTimeout);
	HTTPLog(DDEBUG) << ", client_read_timeout updated to: " << _sock->GetEndTime();
}

void HTTPContext::_parseAndConfig()
{
	try
	{
		if (_router.GetRequest().isComplete(_buf, _readLen))
			_status = HTTPContext::eWritingResponse;
		HTTPLog(DEBUG) << "_parseAndConfig, " << _status;
		if (!_isMaxBodyInit && _router.GetRequest().getHost() != "")
		{
			HTTPLog(DEBUG) << ", host detected: [" << _router.GetRequest().getHost() << "], initializing max body size.";
			_setMaxBodySize();
		}
	}
	catch (exception &e)
	{
		_errNo = e.what();
		_status = HTTPContext::eWriteError;
	}
}

void HTTPContext::_handleRequest()
{
	HTTPLog(DDEBUG)
		<< ", _handleRequest: _status=" << _status;
	if (_sock->IsTimeOut())
	{
		_status = HTTPContext::eWritingResponse;
		return;
	}
	_readFromSocket();
	if (_status == HTTPContext::eDelete)
	{
		Utility::ReleaseBuffer(_buf);
		_buf = NULL;
		return;
	}
	_parseAndConfig();
	if (_status >= HTTPContext::eWritingResponse)
	{
		INFO() << Socket::getRemoteName(_sock->GetFd()) << " " << _router.GetRequest().getMethod() << " " << _router.GetRequest().getPath();
		HTTPLog(DDEBUG) << "request parsing done";
		Utility::ReleaseBuffer(_buf);
		_buf = NULL;
		if (_status == HTTPContext::eWriteError)
		{
			ERR() << "Socket fd: " << _sock->GetFd() << ", error detected, errNo=" << _errNo << ", handling error pages.";
			_repsense.HandleErrorPages(_errNo);
			_status = HTTPContext::eWritingResponse;
		}
	}
}

HTTPContext::HTTPContext(vector<Config::Server> *servers, size_t maxBodySize, ClientSocket *sock) : _sock(sock), _servers(servers)
{
	_router.srv = &((*servers)[0]);
	_router.GetRequest().SetMaxBodySize(maxBodySize);
	_repsense.Init(sock, &(_router));
	_buf = Utility::GetBuffer();
	_isMaxBodyInit = false;
	_status = HTTPContext::eReadingRequest;
	_readLen = 0;
	_multiplexer = Multiplexer::GetCurrentMultiplexer();
}

void HTTPContext::_handleConnectionEnd()
{
	if (_router.GetRequest().isKeepAlive() && _sock->ClosedConnection() == false && _status != HTTPContext::eDelete)
	{
		_sock->SetKeepAlive(true);
		_status = HTTPContext::eDone;
		INFO() << "Socket fd: " << _sock->GetFd() << " marked for keep-alive.";
	}
	else
	{
		INFO() << "Socket fd: " << _sock->GetFd() << " marked for deletion.";
		shutdown(_sock->GetFd(), SHUT_WR);
		if (!_router.GetRequest().isKeepAlive())
		{
			_multiplexer->ScheduleForDeletion(_sock);
		}
		else
		{
			_sock->cleanBody = true;
			_multiplexer->ChangeToEpollIn(_sock);
			_sock->SetTimeout(30);
		}
		Utility::SigPipe = false;
		_status = HTTPContext::eDelete;
		HTTPLog(DDEBUG) << "cleanBody: marking socket fd=" << _sock->GetFd();
	}
}

HTTPContext::~HTTPContext()
{
	DDEBUG("HTTPContext") << "HTTPContext destructor called, sock=" << (_sock ? _sock->GetFd() : -1);

	if (_buf)
	{
		Utility::ReleaseBuffer(_buf);
	}
}

string HTTPContext::_logPrefix()
{
	if (_prefix != "")
		return _prefix;
	stringstream s;
	if (_sock == NULL)
		return "";
	s << "Socket fd: " << _sock->GetFd() << ", ";
	_prefix = s.str();
	return _prefix;
}

int HTTPContext::GetStatus() const
{
	return _status;
}

vector<Config::Server> *HTTPContext::GetServers() const
{
	return _servers;
}

void HTTPContext::SetServers(vector<Config::Server> *servers)
{
	_servers = servers;
}
