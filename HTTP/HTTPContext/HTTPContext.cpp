#include "HTTPContext.hpp"

void HTTPContext::Handle(AFd *fd)
{
	(void)fd;
	HTTPLog(DDEBUG) << "Handle() start";
	if (router.isRequestComplete() == false &&
		sock->isTimeOut() == false && err == false)
		HandleRequest();
	if (router.isRequestComplete() && sock->MarkedToDelete == false)
	{
		if (repsense.HandleResponse())
			MarkedSocketToFree();
	}
	else if (sock->isTimeOut() && sock->MarkedToDelete == false)
		MarkedSocketToFree();
	if (sock->MarkedToDelete == false)
	{
		activeInPipe();
		activeOutPipe();
	}
	else if (sock->isKeepAlive() == false) {
		shutdown(sock->GetFd(), SHUT_WR);
	}
}

int HTTPContext::_readFromSocket()
{
	int len = read(sock->GetFd(), buf, BUF_SIZE);
	sock->UpdateTime();
	HTTPLog(DDEBUG)
		<< "_readFromSocket: read returned "
		<< len;
	if (len <= 0 || Utility::SigPipe)
	{
		HTTPLog(DDEBUG) << "_readFromSocket: connection closed or SigPipe, setting err=true.";
		err = true;
		return 0;
	}
	return len;
}

void HTTPContext::setMaxBodySize()
{
	HTTPLog(DEBUG) << "setMaxBodySize, err: " << err;
	router.srv = &Config::GetServerName(*servers, router.GetRequest().getHost());
	router.CreatePath(router.srv);

	if (router.loc != NULL && router.loc->isMaxBodySize)
	{
		router.GetRequest().SetMaxBodySize(router.loc->clientMaxBodySize);
		HTTPLog(DDEBUG) 
			<< ", max body size set from location: " 
			<< router.GetPath().getLocation()->clientMaxBodySize;
	}
	else if (router.srv->isMaxBodySize)
	{
		router.GetRequest().SetMaxBodySize(router.srv->clientMaxBodySize);
		HTTPLog(DDEBUG) 
			<< ", max body size set from server: " 
			<< router.srv->clientMaxBodySize;
	}
	isMaxBodyInit = true;
	if (router.GetPath().isCGI())
		sock->setTimeout(router.srv->cgiTimeout);
	else
		sock->setTimeout(router.srv->clientReadTimeout);
	HTTPLog(DDEBUG) << ", client_read_timeout updated to: " << sock->GetEndTime();
}

bool HTTPContext::_parseAndConfig(int len)
{
	try
	{
		if (err)
			return true;
		bool complete = router.GetRequest().isComplete(buf, len);
		HTTPLog(DEBUG) << "_parseAndConfig, " << complete;
		if (!isMaxBodyInit && router.GetRequest().getHost() != "")
		{
			HTTPLog(DEBUG) << ", host detected: [" << router.GetRequest().getHost() << "], initializing max body size.";
			setMaxBodySize();
		}
		return complete;
	}
	catch (exception &e)
	{
		ERR() << "Socket fd: " << sock->GetFd() << "_parseAndConfig, exception: " << e.what();
		errNo = e.what();
		err = true;
		return true;
	}
}

void HTTPContext::_setupPipeline()
{
	if (in)
		return;
	HTTPLog(DDEBUG) << ", _setupPipeline: setting up pipes and switching epoll state.";

	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();

	sock->setupPipes();
	if (sock->MarkedToDelete)
		return;

	in = new SocketPipe(sock->pipefd[0], sock);
	MulObj->AddAsEpollIn(in);

	out = new SocketPipe(sock->pipefd[1], sock);
	MulObj->AddAsEpollOut(out);
	HTTPLog(DDEBUG) 
		<< ", _setupPipeline complete: in_pipe_fd=" 
		<< sock->pipefd[0] << ", out_pipe_fd=" << sock->pipefd[1];
}

void HTTPContext::HandleRequest()
{
	HTTPLog(DDEBUG)
			<< ", Handle() router.isRequestComplete(): "
			<< router.isRequestComplete() << ", err: " << err;
	int len = _readFromSocket();
	if (len <= 0 && !err)
		return;
	else if (err) {
		MarkedSocketToFree();
		sock->setKeepAlive(false);
		return;
	}
	bool isComplete = _parseAndConfig(len);

	if (isComplete)
	{
		INFO() << Socket::getRemoteName(sock->GetFd()) << " " << router.GetRequest().getMethod() << " " << router.GetRequest().getPath();
		HTTPLog(DDEBUG) << "request parsing done";

		if (router.GetPath().isCGI()) {
			_setupPipeline();
		}
		router.SetRequestComplete();
		if (err)
		{
			ERR() << "Socket fd: " << sock->GetFd() << ", error detected, errNo=" << errNo << ", handling error pages.";
			if (Config::GetErrorPath(*router.srv, errNo) != "" || StaticFile::GetFileByName(errNo) != NULL)
				repsense.HandelErrorPages(errNo);
			else
				repsense.HandelErrorPages("400");
		}
	}
}

HTTPContext::HTTPContext(vector<Config::Server > *servers, size_t maxBodySize, SocketIO *sock): sock(sock), servers(servers)
{
	in = NULL;
	out = NULL;
	buf = NULL;
	err = false;
	router.srv = &((*servers)[0]);
	router.GetRequest().SetMaxBodySize(maxBodySize);
	repsense.Init(sock, &(router));
	buf = Utility::GetBuffer();
	isMaxBodyInit = false;
}

void HTTPContext::MarkedSocketToFree()
{
	INFO() << "Client " << Socket::getRemoteName(sock->GetFd()) << " marking for deletion";
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();

	if (router.GetRequest().isKeepAlive() && sock->closeConnection == false)
		sock->setKeepAlive(true);
	else {
		sock->cleanBody = true;
		MulObj->ChangeToEpollIn(sock);
		HTTPLog(DDEBUG) << "cleanBody: marking socket fd=" << sock->GetFd();
	}

	Utility::SigPipe = false;
	if (in != NULL)
		MulObj->ChangeToEpollOneShot(in);
	if (out != NULL)
		MulObj->ChangeToEpollOneShot(out);
	sock->MarkedToDelete = true;
}

HTTPContext::~HTTPContext()
{
	DDEBUG("HTTPContext") << "HTTPContext destructor called, sock=" << (sock ? sock->GetFd() : -1);
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();
	bool res;

	if (in)
	{
		res = MulObj->DeleteFromEpoll(in);
		DDEBUG("HTTPContext") << "  -> Deleting in-pipe fd=" << in->GetFd() << ", deleted: " << res;
		delete in;
	}
	if (out)
	{
		res = MulObj->DeleteFromEpoll(out);
		DDEBUG("HTTPContext") << "  -> Deleting out-pipe fd=" << out->GetFd() << ", deleted: " << res;
		delete out;
	}
	Utility::ReleaseBuffer(buf);
}

void HTTPContext::activeInPipe()
{
	if (in == NULL)
		return;
	HTTPLog(DDEBUG) << ", activeInPipe: activating in-pipe fd=" << in->GetFd();
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();

	MulObj->ChangeToEpollIn(in);
}

void HTTPContext::activeOutPipe()
{
	if (out == NULL)
		return;
	HTTPLog(DDEBUG) << ", activeOutPipe: activating out-pipe fd=" << out->GetFd();
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();

	MulObj->ChangeToEpollOut(out);
}

string HTTPContext::logPrefix() 
{ 
	stringstream s;
	if (sock == NULL)
		return "";
	s << "Socket fd: " << sock->GetFd() << ", ";
	return s.str(); 
}
