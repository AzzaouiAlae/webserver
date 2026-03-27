#include "HTTPContext.hpp"

void HTTPContext::Handle(AFd *fd)
{
	(void)fd;
	HTTPLog(DDEBUG) << "Handle() start";
	if (_status == HTTPContext::eReadingRequest) {
		_handleRequest();
	}
	if (_status == HTTPContext::eWritingResponse)
	{
		if (_repsense.HandleResponse()) 
			_status = HTTPContext::eDone;
	}
	if (_status == HTTPContext::eWritingResponse)
	{
		_activeInPipe();
		_activeOutPipe();
	}
	if (_status >= HTTPContext::eDone) {
		HTTPLog(DDEBUG) << "Handle() done, marking socket for deletion.";
		_markedSocketToFree();
	}
	if (_status == HTTPContext::eDelete) {
		HTTPLog(DDEBUG) << "Handle() marked for deletion, closing socket.";
		shutdown(_sock->GetFd(), SHUT_WR);
		_sock->setKeepAlive(false);
	}
	
}

void HTTPContext::_readFromSocket()
{
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
	_router.srv = &Config::GetServerName(*servers, _router.GetRequest().getHost());
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
		_sock->setTimeout(_router.srv->cgiTimeout);
	else
		_sock->setTimeout(_router.srv->clientReadTimeout);
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

void HTTPContext::_setupPipeline()
{
	if (_in)
		return;
	HTTPLog(DDEBUG) << ", _setupPipeline: setting up pipes and switching epoll state.";

	_sock->setupPipes();
	if (_sock->MarkedToDelete)
		return;

	_in = new SocketPipe(_sock->pipefd[0], _sock);
	_multiplexer->AddAsEpollIn(_in);

	_out = new SocketPipe(_sock->pipefd[1], _sock);
	_multiplexer->AddAsEpollOut(_out);
	HTTPLog(DDEBUG) 
		<< ", _setupPipeline complete: in_pipe_fd=" 
		<< _sock->pipefd[0] << ", out_pipe_fd=" << _sock->pipefd[1];
}

void HTTPContext::_handleRequest()
{
	HTTPLog(DDEBUG)
			<< ", Handle() router.isRequestComplete(): _status=" << _status;
	if (_sock->isTimeOut()) {
		_status = HTTPContext::eWritingResponse;
		return;
	}
	_readFromSocket();
	if (_status == HTTPContext::eDelete) {
		return;
	}
	_parseAndConfig();
	if (_status >= HTTPContext::eWritingResponse) 
	{
		INFO() << Socket::getRemoteName(_sock->GetFd()) << " " << _router.GetRequest().getMethod() << " " << _router.GetRequest().getPath();
		HTTPLog(DDEBUG) << "request parsing done";

		if (_router.GetPath().isCGI() || _router.GetRequest().getMethod() == "POST") {
			_setupPipeline();
		}
		if (_status == HTTPContext::eWriteError)
		{
			ERR() << "Socket fd: " << _sock->GetFd() << ", error detected, errNo=" << _errNo << ", handling error pages.";
			_repsense.HandelErrorPages(_errNo);
			_status = HTTPContext::eWritingResponse;
		}
	}
}

HTTPContext::HTTPContext(vector<Config::Server > *servers, size_t maxBodySize, SocketIO *sock): _sock(sock), servers(servers)
{
	_in = NULL;
	_out = NULL;
	_router.srv = &((*servers)[0]);
	_router.GetRequest().SetMaxBodySize(maxBodySize);
	_repsense.Init(sock, &(_router));
	_buf = Utility::GetBuffer();
	_isMaxBodyInit = false;
	_status = HTTPContext::eReadingRequest;
	_readLen = 0;
	_multiplexer = Multiplexer::GetCurrentMultiplexer();
}

void HTTPContext::_markedSocketToFree()
{
	INFO() << "Client " << Socket::getRemoteName(_sock->GetFd()) << " marking for deletion";

	if (_router.GetRequest().isKeepAlive() && _sock->closeConnection == false)
		_sock->setKeepAlive(true);
	else {
		_status = HTTPContext::eDelete;
		_sock->cleanBody = true;
		_multiplexer->ChangeToEpollIn(_sock);
		HTTPLog(DDEBUG) << "cleanBody: marking socket fd=" << _sock->GetFd();
	}

	Utility::SigPipe = false;
	if (_in != NULL)
		_multiplexer->ChangeToEpollOneShot(_in);
	if (_out != NULL)
		_multiplexer->ChangeToEpollOneShot(_out);
	_sock->MarkedToDelete = true;
}

HTTPContext::~HTTPContext()
{
	DDEBUG("HTTPContext") << "HTTPContext destructor called, sock=" << (_sock ? _sock->GetFd() : -1);
	bool res;

	if (_in)
	{
		res = _multiplexer->DeleteFromEpoll(_in);
		DDEBUG("HTTPContext") << "  -> Deleting in-pipe fd=" << _in->GetFd() << ", deleted: " << res;
		delete _in;
	}
	if (_out)
	{
		res = _multiplexer->DeleteFromEpoll(_out);
		DDEBUG("HTTPContext") << "  -> Deleting out-pipe fd=" << _out->GetFd() << ", deleted: " << res;
		delete _out;
	}
	Utility::ReleaseBuffer(_buf);
}

void HTTPContext::_activeInPipe()
{
	if (_in == NULL)
		return;
	HTTPLog(DDEBUG) << ", activeInPipe: activating in-pipe fd=" << _in->GetFd();
	_multiplexer->ChangeToEpollIn(_in);
}

void HTTPContext::_activeOutPipe()
{
	if (_out == NULL)
		return;
	HTTPLog(DDEBUG) << ", activeOutPipe: activating out-pipe fd=" << _out->GetFd();
	_multiplexer->ChangeToEpollOut(_out);
}

string HTTPContext::_logPrefix() 
{ 
	stringstream s;
	if (_sock == NULL)
		return "";
	s << "Socket fd: " << _sock->GetFd() << ", ";
	return s.str(); 
}