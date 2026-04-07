#include "ConnectionContext.hpp"
#include "HTTPContext.hpp"
#include "NetIO.hpp"
#include "Socket.hpp"
#include "Config.hpp"

void ConnectionContext::Handle(AFd *fd)
{
	int max = Utility::maxFds;
	Socket *sock = (Socket *)fd;
	
	if ((int)Singleton::GetFds().size() >= max || sock->acceptedSocket >= max) {
		NetIO::CloseTheOldestSocket();
	}
	ClientSocket *sockIO = new ClientSocket(sock->acceptedSocket, Config::GetMaxClientReadTimeout(*servers));
	Singleton::GetFds().insert(sockIO);
	sockIO->context = new HTTPContext(servers, Config::GetMaxBodySize(*servers), sockIO);
	Multiplexer::GetCurrentMultiplexer()->AddAsEpollIn(sockIO);
	INFO()<< "New client connected from " << Socket::getRemoteName(sockIO->GetFd());
}

void ConnectionContext::HandleKeepAlive(AFd *fd)
{
	ClientSocket *sock = (ClientSocket *)fd;
	HTTPContext *ctx = static_cast<HTTPContext *>(sock->context);
	if (ctx->GetStatus() == HTTPContext::eDone && sock->IsKeepAlive())
	{
		vector<Config::Server> *servers = ctx->GetServers();
		delete sock->context;
		int keepAliveTimeout = Config::GetServerName(*servers, "").keepAliveTimeout;
		sock->SetTimeout(keepAliveTimeout);
		sock->UpdateTime();
		sock->context = new HTTPContext(servers, Config::GetMaxBodySize(*servers), sock);
		Multiplexer::GetCurrentMultiplexer()->ChangeToEpollIn(sock);
	}
}

ConnectionContext::ConnectionContext()
{}

ConnectionContext::~ConnectionContext()
{}