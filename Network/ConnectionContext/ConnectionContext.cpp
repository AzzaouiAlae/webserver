#include "ConnectionContext.hpp"
#include "HTTPContext.hpp"
#include "SocketIO.hpp"
#include "Socket.hpp"


void ConnectionContext::Handle(AFd *fd)
{
	Socket *sock = (Socket *)fd;
	SocketIO *sockIO = new SocketIO(sock->acceptedSocket);
	Singleton::GetFds().insert(fd);
	sockIO->context = new HTTPContext(servers, Config::GetMaxBodySize(*servers), sockIO);
	Multiplexer::GetCurrentMultiplexer()->AddAsEpollIn(sockIO);
	INFO() << "New client connected from " << Socket::getRemoteName(sockIO->GetFd());
}

ConnectionContext::ConnectionContext()
{}
ConnectionContext::~ConnectionContext()
{}