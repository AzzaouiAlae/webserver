#include "HTTPContext.hpp"

void HTTPContext::Handle(AFd *fd)
{
	DDEBUG("HTTPContext") << "Handle(AFd) called, fd=" << fd->GetFd() << ", type=" << fd->GetType();
	if (fd->GetType() == "Socket")
		Handle((Socket *)fd);
	else if (fd->GetType() == "SocketIO")
		Handle();
}

void HTTPContext::Handle(Socket *sock)
{
	DDEBUG("HTTPContext") << "Handle(Socket) called, accepting new connection on socket fd=" << sock->GetFd();
	SocketIO *fd = new SocketIO(sock->acceptedSocket);
	Singleton::GetFds().insert(fd);
	servers = &((Singleton::GetVirtualServers())[sock->GetFd()]);
	fd->context = new HTTPContext();
	((HTTPContext *)fd->context)->servers = servers;
	((HTTPContext *)fd->context)->router.srv = &((*servers)[0]);
	((HTTPContext *)fd->context)->router.GetRequest().SetMaxBodySize(Config::GetMaxBodySize(*servers));
	((HTTPContext *)fd->context)->sock = fd;
	((HTTPContext *)fd->context)->repsense.Init(fd, &(((HTTPContext *)fd->context)->router));
	Multiplexer::GetCurrentMultiplexer()->AddAsEpollIn(fd);
	DDEBUG("HTTPContext") << "  -> New SocketIO fd=" << fd->GetFd() << " created and registered as EPOLLIN.";
}

void HTTPContext::Handle()
{
	DEBUG("HTTPContext") << "Socket fd: " << sock->GetFd() << ", HTTPContext::Handle() start";
	if (router.isRequestComplete() == false && err == false)
	{
		DEBUG("HTTPContext") << "Socket fd: " << sock->GetFd() << ", HTTPContext::Handle() router.isRequestComplete(): " <<
		router.isRequestComplete() << ", err: " << err;

		HandleRequest();
	}
	else
	{
		DDEBUG("HTTPContext") << "Socket fd: " << sock->GetFd() << ", request complete, handling response.";
		if (repsense.HandleResponse()) {
			DDEBUG("HTTPContext") << "Socket fd: " << sock->GetFd() << ", response done, marking socket to free.";
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
    if (buf == NULL) {
        buf = (char *)calloc(1, BUF_SIZE + 1);
        DDEBUG("HTTPContext") << "Socket fd: " << sock->GetFd() << ", allocated read buffer (" << BUF_SIZE << " bytes).";
    }
    int len = read(sock->GetFd(), buf, BUF_SIZE); 
	DDEBUG("HTTPContext") << "Socket fd: " << sock->GetFd() << ", _readFromSocket: read returned " << len;
    if (len == 0 || Utility::SigPipe) {
        DDEBUG("HTTPContext") << "Socket fd: " << sock->GetFd() << ", _readFromSocket: connection closed or SigPipe, setting err=true.";
        err = true;
        return 0;
    }
    return len;
}

void HTTPContext::setMaxBodySize()
{
	DEBUG("HTTPContext") << "Socket fd: " << sock->GetFd() << ", HTTPContext::setMaxBodySize, err: " << err;
	router.srv = &Config::GetServerName(*servers, router.GetRequest().getHost());
	router.CreatePath(router.srv);

	if (router.GetPath().getLocation() != NULL && router.GetPath().getLocation()->isMaxBodySize) {
		router.GetRequest().SetMaxBodySize(router.GetPath().getLocation()->clientMaxBodySize);
		DDEBUG("HTTPContext") << "Socket fd: " << sock->GetFd() << ", max body size set from location: " << router.GetPath().getLocation()->clientMaxBodySize;
	} 
	else if (router.srv->isMaxBodySize)
	{
		router.GetRequest().SetMaxBodySize(router.srv->clientMaxBodySize);
		DDEBUG("HTTPContext") << "Socket fd: " << sock->GetFd() << ", max body size set from server: " << router.srv->clientMaxBodySize;
	}
	isMaxBodyInit = true;
}

bool HTTPContext::_parseAndConfig(int len)
{
    try {
        if (err) return true; // If error already exists, treat as "complete" to trigger error page
		
        // 1. Parse buffer
        bool complete = router.GetRequest().isComplete(buf, len);
		DEBUG("HTTPContext") << "Socket fd: " << sock->GetFd() << ", HTTPContext::_parseAndConfig, " << complete;
        // 2. Configure Server Block logic
        // We do this immediately so we can check MaxBodySize during parsing if needed
		if (!isMaxBodyInit && router.GetRequest().getHost() != "") {
			DDEBUG("HTTPContext") << "Socket fd: " << sock->GetFd() << ", host detected: [" << router.GetRequest().getHost() << "], initializing max body size.";
			setMaxBodySize();
		}
        return complete;

    } catch (exception &e) {
		ERR() << "Socket fd: " << sock->GetFd() << ", HTTPContext::_parseAndConfig, exception: " << e.what();
		errNo = e.what();
        err = true;
        return true; // Return true to proceed to error handling
    }
}

void HTTPContext::_setupPipeline()
{
    DDEBUG("HTTPContext") << "Socket fd: " << sock->GetFd() << ", _setupPipeline: setting up pipes and switching epoll state.";

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
    DDEBUG("HTTPContext") << "Socket fd: " << sock->GetFd() << ", _setupPipeline complete: in_pipe_fd=" << sock->pipefd[0] << ", out_pipe_fd=" << sock->pipefd[1];
}

void HTTPContext::HandleRequest()
{
    // 1. Read Data
    int len = _readFromSocket();

	DEBUG("HTTPContext") << "Socket fd: " << sock->GetFd() << ", HTTPContext::HandleRequest(), len: " << len;
    
    // If read failed (-1) or nothing to process, return
    if (len == -1 && !err) {
        return; 
    }

    // 2. Parse Data
    bool isComplete = _parseAndConfig(len);
	DEBUG("HTTPContext") << "Socket fd: " << sock->GetFd() << ", HTTPContext::HandleRequest(), isComplete: " << isComplete 
	<< ", err: " << err;

    // 3. Check for Completion or Errors
    if (isComplete || router.GetRequest().isRequestHeaderComplete())
    {
		DDEBUG("HTTPContext") << "Socket fd: " << sock->GetFd() << ", request parsing done, setting up pipeline.";
		_setupPipeline();
	
        if (err) 
		{
			DDEBUG("HTTPContext") << "Socket fd: " << sock->GetFd() << ", error detected, errNo=" << errNo << ", handling error pages.";
			if (Config::GetErrorPath(*router.srv, errNo) != "" || StaticFile::GetFileByName(errNo) != NULL ) {
				repsense.HandelErrorPages(errNo);
			}
			else {
				repsense.HandelErrorPages("400");
			}
			if (router.GetRequest().getcontentlen() < INT64_MAX) {
			sock->maxToClean = router.GetRequest().getcontentlen() + SAFE_MARGIN;
			}
			else
				sock->maxToClean = router.GetRequest().getcontentlen();
			Multiplexer::GetCurrentMultiplexer()->ChangeToEpollIn(sock);
        }
    }
}

HTTPContext::HTTPContext()
{
	in = NULL;
	out = NULL;
	sock = NULL;
	buf = NULL;
	err = false;
	isMaxBodyInit=  false;
}

void HTTPContext::MarkedSocketToFree()
{
	DDEBUG("HTTPContext") << "Socket fd: " << sock->GetFd() << ", MarkedSocketToFree: shutting down and marking for deletion.";
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
	free(buf);
}

void HTTPContext::activeInPipe()
{	
	if (in == NULL)
		return;
	DDEBUG("HTTPContext") << "Socket fd: " << sock->GetFd() << ", activeInPipe: activating in-pipe fd=" << in->GetFd();
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();

	MulObj->ChangeToEpollIn(in);
}

void HTTPContext::activeOutPipe()
{
	if (out == NULL)
		return;
	DDEBUG("HTTPContext") << "Socket fd: " << sock->GetFd() << ", activeOutPipe: activating out-pipe fd=" << out->GetFd();
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();

	MulObj->ChangeToEpollOut(out);
}
