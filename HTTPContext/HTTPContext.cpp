#include "HTTPContext.hpp"

void HTTPContext::Handle(AFd *fd)
{
	if (fd->GetType() == "Socket")
		Handle((Socket *)fd);
	else if (fd->GetType() == "SocketIO")
		Handle((SocketIO *)fd);
}

void HTTPContext::Handle(Socket *sock)
{
	vector<AFd *> &fds = Singleton::GetFds();
	SocketIO *fd = new SocketIO(sock->acceptedSocket);
	servers = &((Singleton::GetServers())[sock->GetFd()]);
	fd->context = this;
	fds.push_back(fd);
	Multiplexer::GetCurrentMultiplexer()->AddAsEpollIn(fd);
}

void HTTPContext::Handle(SocketIO *sock)
{
	Routing &r = sock->GetRouter();
	
	if (r.isRequestComplete() == false) {
		HandleRequest(sock);
	}
	else {
		HandleResponse(sock);
	}
}

void HTTPContext::HandleRequest(SocketIO *sock)
{
	Routing &r = sock->GetRouter();
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();
	vector<AFd *> fds = Singleton::GetFds();
	AFd *fd;
	char buff[1024 * 64];
	int len = read(sock->GetFd(), buff, 1024 * 64 - 1);

	if (len != -1)
	{
		buff[len] = '\0';
		if (r.GetRequest().isComplete(buff))
		{
			r.SetRequestComplete();
			MulObj->ChangeToEpollOut(sock);

			fd = new Pipe(sock->pipefd[0], sock);
			fds.push_back(fd);
			MulObj->AddAsEpollIn(fd);

			fd = new Pipe(sock->pipefd[1], sock);
			fds.push_back(fd);
			MulObj->AddAsEpollOut(fd);
		}
	}
}

void HTTPContext::HandleResponse(SocketIO *sock)
{
	Request &req = sock->GetRouter().GetRequest();

	if (req.getMethod() == "GET") {
		GetMethod(sock);
	}
}

void HTTPContext::GetMethod(SocketIO *sock) 
{
	(void)sock;
	
}

