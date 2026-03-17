#include "ConnectionContext.hpp"
#include "HTTPContext.hpp"
#include "SocketIO.hpp"
#include "Socket.hpp"
#include "Config.hpp"

void ConnectionContext::Handle(AFd *fd)
{
	int max = Utility::maxFds;
	
	if ((int)Singleton::GetFds().size() > max) {
		SocketIO::closeTheOldestSocket();
	}
	Socket *sock = (Socket *)fd;
	
	SocketIO *sockIO = new SocketIO(sock->acceptedSocket, Config::GetMaxClientReadTimeout(*servers));
	Singleton::GetFds().insert(sockIO);
	sockIO->context = new HTTPContext(servers, Config::GetMaxBodySize(*servers), sockIO);
	Multiplexer::GetCurrentMultiplexer()->AddAsEpollIn(sockIO);
	INFO() << "New client connected from " << Socket::getRemoteName(sockIO->GetFd());
}

ConnectionContext::ConnectionContext()
{}
ConnectionContext::~ConnectionContext()
{}