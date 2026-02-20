#include "HTTPContext.hpp"

void HTTPContext::Handle(AFd *fd)
{
	if (fd->GetType() == "Socket")
		Handle((Socket *)fd);
	else if (fd->GetType() == "SocketIO")
		Handle();
}

void HTTPContext::Handle(Socket *sock)
{
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
	
}

void HTTPContext::Handle()
{
	DEBUG() << "Socket fd: " << sock->GetFd() << ", HTTPContext::Handle() start";
	if (router.isRequestComplete() == false && err == false)
	{
		DEBUG() << "Socket fd: " << sock->GetFd() << ", HTTPContext::Handle() router.isRequestComplete(): " <<
		router.isRequestComplete() << ", err: " << err;

		HandleRequest();
	}
	else
	{
		
		if (repsense.HandleResponse()) {
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
    }
    int len = read(sock->GetFd(), buf, BUF_SIZE); 

    if (len == 0 || Utility::SigPipe) {
        err = true;
        return 0;
    }
    return len;
}

void HTTPContext::setMaxBodySize()
{
	DEBUG() << "Socket fd: " << sock->GetFd() << ", HTTPContext::setMaxBodySize, err: " << err;
	router.srv = &Config::GetServerName(*servers, router.GetRequest().getHost());
	int i = Config::GetLocationIndex(*router.srv, router.GetRequest().getPath());

	if (i != -1 && router.srv->Locations[i].isMaxBodySize) {
		router.GetRequest().SetMaxBodySize(router.srv->Locations[i].clientMaxBodySize);
	} 
	else if (router.srv->isMaxBodySize)
	{
		router.GetRequest().SetMaxBodySize(router.srv->clientMaxBodySize);
	}
	isMaxBodyInit = true;
}

bool HTTPContext::_parseAndConfig(int len)
{
    try {
        if (err) return true; // If error already exists, treat as "complete" to trigger error page
		
        // 1. Parse buffer
        bool complete = router.GetRequest().isComplete(buf, len);
		DEBUG() << "Socket fd: " << sock->GetFd() << ", HTTPContext::_parseAndConfig, " << complete;
        // 2. Configure Server Block logic
        // We do this immediately so we can check MaxBodySize during parsing if needed
		if (!isMaxBodyInit && router.GetRequest().getHost() != "") {
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
    // Log the request details
    

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
}

void HTTPContext::HandleRequest()
{
    // 1. Read Data
    int len = _readFromSocket();

	DEBUG() << "Socket fd: " << sock->GetFd() << ", HTTPContext::HandleRequest(), len: " << len;
    
    // If read failed (-1) or nothing to process, return
    if (len == -1 && !err) {
        return; 
    }

    // 2. Parse Data
    bool isComplete = _parseAndConfig(len);
	DEBUG() << "Socket fd: " << sock->GetFd() << ", HTTPContext::HandleRequest(), isComplete: " << isComplete 
	<< ", err: " << err;

    // 3. Check for Completion or Errors
    if (isComplete || router.GetRequest().isRequestHeaderComplete())
    {
		_setupPipeline();
	
        if (err) 
		{
			if (Config::GetErrorPath(*router.srv, errNo) != "" || StaticFile::GetFileByName(errNo) != NULL ) {
				repsense.HandelErrorPages(errNo);
			}
			else {
				repsense.HandelErrorPages("400");
			}
			if (router.GetRequest().getContentLen() < INT64_MAX) {
			sock->maxToClean = router.GetRequest().getContentLen() + SAFE_MARGIN;
			}
			else
				sock->maxToClean = router.GetRequest().getContentLen();
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
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();

	if (in)
	{
		MulObj->DeleteFromEpoll(in);
		delete in;
	}
	if (out)
	{
		MulObj->DeleteFromEpoll(out);
		delete out;
	}
	free(buf);
}

void HTTPContext::activeInPipe()
{	
	if (in == NULL)
		return;
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();

	MulObj->ChangeToEpollIn(in);
}

void HTTPContext::activeOutPipe()
{
	if (out == NULL)
		return;
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();

	MulObj->ChangeToEpollOut(out);
}
