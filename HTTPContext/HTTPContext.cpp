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
	((HTTPContext *)fd->context)->sock = fd;
	Multiplexer::GetCurrentMultiplexer()->AddAsEpollIn(fd);
	Logging::Debug() << "Socket FD: " << sock->GetFd()
					<< " accepte new connection as socket fd: " << sock->acceptedSocket;
}

void HTTPContext::Handle()
{
	repsense.Init(sock, &router);
	if (router.isRequestComplete() == false && err == false)
	{
		Logging::Debug() << "Socket FD: " << sock->GetFd() << " start handle request";
		HandleRequest();
	}
	else
	{
		Logging::Debug() << "Socket FD: " << sock->GetFd() << " start handle response";
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

void HTTPContext::HandleRequest()
{
	int len = 0;
	bool isComplete;
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();
	if (buf == NULL)
	{
		buf = (char *)calloc(1, BUF_SIZE + 1);
	}
	len = read(sock->GetFd(), buf, BUF_SIZE);
	Logging::Debug() << "Socket FD: " << sock->GetFd() << " read " << len << " byte";
	if (len == 0 || Utility::SigPipe) {
		err = true;
		isComplete = true;
	}
	if (len != -1)
	{
		try {
			if (err == false) {
				isComplete = router.GetRequest().isComplete(buf);
			}
		} catch (exception &e)  {
			err = true;
			isComplete = true;
		}
		if (isComplete)
		{
			Logging::Debug() << "Read of Request from Socket fd: " << sock->GetFd() << " Complete";
			Logging::Debug() << "Request is : " << router.GetRequest().getMethod() << " " << router.GetRequest().getPath() << "\n"
			<< router.GetRequest().GetRequest();
			
			router.srv = &Config::GetServerName(*servers, router.GetRequest().getPath());
			
			router.SetRequestComplete();
			MulObj->ChangeToEpollOut(sock);

			in = new Pipe(sock->pipefd[0], sock);
			MulObj->AddAsEpollIn(in);

			out = new Pipe(sock->pipefd[1], sock);
			MulObj->AddAsEpollOut(out);
		}
		if (err) {
			repsense.HandelErrorPages("400");
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
}

void HTTPContext::MarkedSocketToFree()
{
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();

	Utility::SigPipe = false;
	if (in != NULL)
		MulObj->ChangeToEpollOneShot(in);
	if (in != NULL)
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