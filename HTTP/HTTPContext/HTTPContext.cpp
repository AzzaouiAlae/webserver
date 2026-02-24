#include "HTTPContext.hpp"

vector<char *> HTTPContext::buffPoll;

void HTTPContext::Handle(AFd *fd)
{
	HTTPLog(DDEBUG) << ", Handle(AFd) type: " << fd->GetType();
	if (fd->GetType() == "Socket")
		Handle((Socket *)fd);
	else if (fd->GetType() == "SocketIO")
		Handle();
}

void HTTPContext::Handle(Socket *sock)
{
	HTTPLog(DEBUG) << "Handle(Socket) called, accepting new connection on socket fd: " << sock->GetFd();
	SocketIO *fd = new SocketIO(sock->acceptedSocket);
	Singleton::GetFds().insert(fd);
	servers = &((Singleton::GetVirtualServers())[sock->GetFd()]);
	fd->context = new HTTPContext();
	HTTPContext *cont = (HTTPContext *)fd->context;
	cont->servers = servers;
	cont->router.srv = &((*servers)[0]);
	cont->router.GetRequest().SetMaxBodySize(Config::GetMaxBodySize(*servers));
	cont->sock = fd;
	cont->repsense.Init(fd, &(((HTTPContext *)fd->context)->router));
	Multiplexer::GetCurrentMultiplexer()->AddAsEpollIn(fd);
	INFO() << "New client connected from " << Socket::getRemoteName(fd->GetFd());
}

void HTTPContext::Handle()
{
	HTTPLog(DDEBUG) << ", Handle() start";
	if (router.isRequestComplete() == false &&
		sock->isTimeOut() == false && err == false)
	{
		HTTPLog(DDEBUG)
			<< ", Handle() router.isRequestComplete(): "
			<< router.isRequestComplete() << ", err: " << err;
		HandleRequest();
	}
	else
	{
		if (sock->isTimeOut())
		{
			_setupPipeline();
		}
		router.SetRequestComplete();
		HTTPLog(DDEBUG) << ", request complete, handling response.";
		if (repsense.HandleResponse())
		{
			HTTPLog(DDEBUG) << ", response done, marking socket to free.";
			MarkedSocketToFree();
		}
	}
	if (sock->MarkedToDelete == false)
	{
		activeInPipe();
		activeOutPipe();
	}
}



int HTTPContext::_readFromSocket()
{
	if (buf == NULL)
	{
		buf = (char *)calloc(1, BUF_SIZE + 1);
		HTTPLog(DDEBUG)
			<< ", allocated read buffer (" << BUF_SIZE
			<< " bytes).";
	}
	int len = read(sock->GetFd(), buf, BUF_SIZE);
	HTTPLog(DDEBUG)
		<< ", _readFromSocket: read returned "
		<< len;
	if (len <= 0 || Utility::SigPipe)
	{
		HTTPLog(DDEBUG) << ", _readFromSocket: connection closed or SigPipe, setting err=true.";
		err = true;
		return 0;
	}
	return len;
}

void HTTPContext::setMaxBodySize()
{
	HTTPLog(DEBUG) << ", HTTPContext::setMaxBodySize, err: " << err;
	router.srv = &Config::GetServerName(*servers, router.GetRequest().getHost());
	router.CreatePath(router.srv);

	if (router.GetPath().getLocation() != NULL && router.GetPath().getLocation()->isMaxBodySize)
	{
		router.GetRequest().SetMaxBodySize(router.GetPath().getLocation()->clientMaxBodySize);
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
}

bool HTTPContext::_parseAndConfig(int len)
{
	try
	{
		if (err)
			return true; // If error already exists, treat as "complete" to trigger error page

		// 1. Parse buffer
		bool complete = router.GetRequest().isComplete(buf, len);
		HTTPLog(DEBUG) << ", HTTPContext::_parseAndConfig, " << complete;
		// 2. Configure Server Block logic
		// We do this immediately so we can check MaxBodySize during parsing if needed
		if (!isMaxBodyInit && router.GetRequest().getHost() != "")
		{
			HTTPLog(DEBUG) << ", host detected: [" << router.GetRequest().getHost() << "], initializing max body size.";
			setMaxBodySize();
		}
		return complete;
	}
	catch (exception &e)
	{
		ERR() << "Socket fd: " << sock->GetFd() << ", HTTPContext::_parseAndConfig, exception: " << e.what();
		errNo = e.what();
		err = true;
		return true; // Return true to proceed to error handling
	}
}

void HTTPContext::_setupPipeline()
{
	HTTPLog(DDEBUG) << ", _setupPipeline: setting up pipes and switching epoll state.";

	router.SetRequestComplete();

	// Switch Multiplexer state
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();
	MulObj->ChangeToEpollOut(sock);

	// Create Pipes
	// Note: Ensure you manage memory for 'in' and 'out' properly (e.g., delete in destructor)
	in = new Pipe(sock->pipefd[0], sock);
	MulObj->AddAsEpollIn(in);

	out = new Pipe(sock->pipefd[1], sock);
	MulObj->AddAsEpollOut(out);
	HTTPLog(DDEBUG) 
		<< ", _setupPipeline complete: in_pipe_fd=" 
		<< sock->pipefd[0] << ", out_pipe_fd=" << sock->pipefd[1];
}

void HTTPContext::HandleRequest()
{
	// 1. Read Data
	int len = _readFromSocket();
	string buff = "";
	if (len > 0)
	{
		buff.append(buf, len);
	}
	HTTPLog(DEBUG) << ", HTTPContext::HandleRequest(), len: " << len;
	// If read failed (-1) or nothing to process, return
	if (len == -1 && !err)
	{
		return;
	}
	HTTPLog(DDEBUG) << "\n" << buff;
	// 2. Parse Data
	bool isComplete = _parseAndConfig(len);
	HTTPLog(DDEBUG)
		<< ", HTTPContext::HandleRequest(), isComplete: "
		<< isComplete << ", err: " << err;

	// 3. Check for Completion or Errors
	if (isComplete || router.GetRequest().isRequestHeaderComplete())
	{
		INFO() << Socket::getRemoteName(sock->GetFd()) << " " << router.GetRequest().getMethod() << " " << router.GetRequest().getPath();
		HTTPLog(DDEBUG) << ", request parsing done, setting up pipeline.";

		_setupPipeline();

		if (err)
		{
			ERR() << "Socket fd: " << sock->GetFd() << ", error detected, errNo=" << errNo << ", handling error pages.";
			if (Config::GetErrorPath(*router.srv, errNo) != "" || StaticFile::GetFileByName(errNo) != NULL)
			{
				repsense.HandelErrorPages(errNo);
			}
			else
			{
				repsense.HandelErrorPages("400");
			}
			if (router.GetRequest().getcontentlen() < INT64_MAX)
			{
				sock->maxToClean = router.GetRequest().getcontentlen() + SAFE_MARGIN;
			}
			else
				sock->maxToClean = router.GetRequest().getcontentlen();
			Multiplexer::GetCurrentMultiplexer()->ChangeToEpollIn(sock);
		}
	}
}

void HTTPContext::ClearBuffPoll()
{
	for(size_t i = 0; i < buffPoll.size(); i++)
	{
		free(buffPoll[i]);
	}
}

HTTPContext::HTTPContext()
{
	in = NULL;
	out = NULL;
	sock = NULL;
	buf = NULL;
	err = false;
	isMaxBodyInit = false;
	if (buffPoll.size() > 0) {
		buf = buffPoll[buffPoll.size() - 1];
		buffPoll.pop_back();
	}
}

void HTTPContext::MarkedSocketToFree()
{
	INFO() << "Client " << Socket::getRemoteName(sock->GetFd()) << " disconnected";
	HTTPLog(DDEBUG) << ", MarkedSocketToFree: shutting down and marking for deletion.";
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();

	Utility::SigPipe = false;
	if (in != NULL)
		MulObj->ChangeToEpollOneShot(in);
	if (out != NULL)
		MulObj->ChangeToEpollOneShot(out);
	shutdown(sock->GetFd(), SHUT_RDWR);
	sock->MarkedToDelete = true;
}

HTTPContext::~HTTPContext()
{
	DDEBUG("HTTPContext") << "HTTPContext destructor called, sock=" << (sock ? sock->GetFd() : -1);
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();

	if (in)
	{
		DDEBUG("HTTPContext") << "  -> Deleting in-pipe fd=" << in->GetFd();
		MulObj->DeleteFromEpoll(in);
		delete in;
	}
	if (out)
	{
		DDEBUG("HTTPContext") << "  -> Deleting out-pipe fd=" << out->GetFd();
		MulObj->DeleteFromEpoll(out);
		delete out;
	}
	if (buffPoll.size() > 100)
		free(buf);
	else
		buffPoll.push_back(buf);
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