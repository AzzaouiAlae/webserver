#include "Socket.hpp"
#include "../SocketIO/SocketIO.hpp"
#include "../HTTPContext/HTTPContext.hpp"

int Socket::errorNumber = 0;

void Socket::Handle()
{
	acceptedSocket = accept4(fd, NULL, NULL, SOCK_NONBLOCK);
	DDEBUG("Socket") << "Handle: accepted new connection, fd=" << acceptedSocket << " on listener fd=" << fd;
	context->Handle(this);
}

int Socket::inetConnect(const string &host, const string &service, int type)
{
	addrinfo hints, *result, *rp;
	char *h = NULL, *s = NULL;
	int sock = -1;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = type;

	if (host != "")
		h = (char *)host.c_str();
	if (service != "")
		s = (char *)service.c_str();

	if (getaddrinfo(h, s, &hints, &result) != 0)
	{
		DDEBUG("Socket") << "inetConnect: getaddrinfo failed for host='" << host << "', service='" << service << "'";
		return -1;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next)
	{
		sock = socket(rp->ai_family, rp->ai_socktype | O_NONBLOCK, rp->ai_protocol);
		if (sock == -1)
			continue;

		if (connect(sock, rp->ai_addr, rp->ai_addrlen) != -1)
			break;
		
		close(sock);
	}
	freeaddrinfo(result);
	DDEBUG("Socket") << "inetConnect: host='" << host << "', service='" << service << "', result fd=" << ((rp != NULL) ? sock : -1);
	return (rp != NULL) ? sock : -1;
}

int Socket::inetPassiveSocket(const char *host, const char *service, int type,
							bool doListen, int backlog)
{
	addrinfo hints, *result, *rp;
	int sock = 0, optVal = 1, res;
	(void)res;
	(void)optVal;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = type;
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(host, service, &hints, &result) != 0)
		return -1;

	for (rp = result; rp != NULL; rp = rp->ai_next)
	{
		sock = socket(rp->ai_family, rp->ai_socktype | O_NONBLOCK, rp->ai_protocol);
		if (sock == -1)
			continue;
		if (doListen)
		{
			if (rp->ai_family != AF_INET6)
				res = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
								&optVal, sizeof(optVal));
			else
				res = setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY,
								&optVal, sizeof(optVal));
			if (res != 0)
			{
				close(sock);
				freeaddrinfo(result);
				return -1;
			}
		}
		if (bind(sock, rp->ai_addr, rp->ai_addrlen) == 0)
			break;
		if (sock > 0)
			close(sock);
		sock = -1;
		break;
	}
	freeaddrinfo(result);
	if (doListen && rp != NULL && sock > 0)
	{
		if (listen(sock, backlog) == -1)
		{
			close(sock);
			return -1;
		}
	}
	return (rp != NULL) ? sock : -1;
}

int Socket::inetListen(const string &host, const string &service, int backlog)
{
	const char *h;
	if (host == "")
		h = NULL;
	else
		h = host.c_str();
	int result = inetPassiveSocket(h, service.c_str(), SOCK_STREAM, true, backlog);
	DDEBUG("Socket") << "inetListen: host='" << host << "', port='" << service << "', backlog=" << backlog << ", result fd=" << result;
	return result;
}

int Socket::inetBind(const string &host, const string &service, int type)
{
	return inetPassiveSocket(host.c_str(), service.c_str(), type, false, 0);
}

string Socket::inetAddressStr(sockaddr *addr, socklen_t addrlen)
{
	stringstream addrStr;
	char host[NI_MAXHOST], service[NI_MAXSERV];
	errorNumber = 0;

	if (getnameinfo(addr, addrlen, host, NI_MAXHOST, service,
					NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV) == 0)
		addrStr << "(" << host << ", " << service << ")";
	else
		addrStr << "UNKNOWN";

	errorNumber = 1;
	return addrStr.str();
}

void Socket::inetAddressStr(sockaddr *addr, socklen_t addrlen, string &host, string &port)
{
	stringstream addrStr;
	char _host[NI_MAXHOST], service[NI_MAXSERV];
	errorNumber = 0;

	if (getnameinfo(addr, addrlen, _host, NI_MAXHOST, service,
					NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV) == 0)
	{
		host = _host;
		port = service;
	}
	else
		errorNumber = 1;
}

sockaddr_storage Socket::getLocalAddress(int sock)
{
	sockaddr_storage addr;
	socklen_t len = sizeof(addr);
	errorNumber = 0;

	if (getsockname(sock, (sockaddr *)&addr, &len) == -1)
		errorNumber = 1;
	return addr;
}

string Socket::getLocalName(int sock)
{
	sockaddr_storage addr = getLocalAddress(sock);

	return inetAddressStr((sockaddr *)&addr, sizeof(addr));
}

sockaddr_storage Socket::getRemoteAddress(int sock)
{
	sockaddr_storage addr;
	socklen_t len = sizeof(addr);
	errorNumber = 0;

	if (getpeername(sock, (sockaddr *)&addr, &len) == -1)
		errorNumber = 1;
	return addr;
}

string Socket::getRemoteName(int sock)
{
	sockaddr_storage addr = getRemoteAddress(sock);

	return inetAddressStr((sockaddr *)&addr, sizeof(addr));
}

void Socket::getRemoteName(int sock, string &host, string &port)
{
	sockaddr_storage addr = getRemoteAddress(sock);

	inetAddressStr((sockaddr *)&addr, sizeof(addr), host, port);
}

void Socket::getLocalName(int sock, string &host, string &port)
{
	sockaddr_storage addr = getLocalAddress(sock);

	inetAddressStr((sockaddr *)&addr, sizeof(addr), host, port);
}

int Socket::getSocketType(int sock)
{
	int optval;
	socklen_t optlen;

	optlen = sizeof(optval);
	if (getsockopt(sock, SOL_SOCKET, SO_TYPE, &optval, &optlen) == -1)
		return -1;
	return optval;
}

Socket::Socket(int sock): ISocket(sock, "Socket")
{
	DEBUG("Socket") << "Socket created, fd=" << sock;
}

string Socket::getIpByHost(const string &host, const string &port, int type)
{
    addrinfo hints, *result;
    string ip = "";
    string Port = ""; 
	const char *h = NULL, *p = NULL;

	if (host != "")
		h = host.c_str();
	if (port != "")
		p = port.c_str();

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = type;     
    hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(h, p, &hints, &result) != 0)
        return "";

    if (result != NULL) {
        inetAddressStr(result->ai_addr, result->ai_addrlen, ip, Port);
    }
	else {
		return "";
	}

    freeaddrinfo(result);
    return ip + ":" + Port;
}

void Socket::AddSocketNew(string &host, string &port, Config::Server srv)
{
	map<int, vector<Config::Server> > &sockets = Singleton::GetVirtualServers();
	INFO() << "Attempting to bind listening socket on " << host << ":" << port;
	int sock = inetListen(host.c_str(), port.c_str(), 1000);
	set<AFd *> &fds = Singleton::GetFds();

	if (sock != -1) 
	{
		INFO() << "Socket successfully bound on " << host << ":" << port << " (fd=" << sock << ")";
		Socket *NewFd = new Socket(sock);
		NewFd->context = new HTTPContext();
		fds.insert(NewFd);
		vector<Config::Server> myVector;
		myVector.push_back(srv);
		sockets[sock] = myVector;
		
	}
	else  {
		DEBUG("Socket") << "Bind failed for " << host << ":" << port << ", searching for existing socket to share.";
		FindServerNew(host, port, srv);
	}
}

void Socket::FindServerNew(string &host, string &port, Config::Server srv)
{
    map<int, vector<Config::Server> > &sockets = Singleton::GetVirtualServers();
    stringstream strError;
    strError << "Bind to address: " << host << ":" << port << " fail";

    // 1. If no sockets exist at all, the port is locked by another program (e.g. Nginx/Apache)
    if (sockets.empty()) {
        Error::ThrowError(strError.str().c_str());
    }

    // 2. Resolve the Target Host ONCE
    string targetV4 = getIpByHost(host, port, AF_INET);
    string targetV6 = getIpByHost(host, port, AF_INET6);

    if (targetV4.empty() && targetV6.empty()) {
        Error::ThrowError(strError.str().c_str());
    }

    // 3. Iterate existing sockets to find a match
    for (map<int, vector<Config::Server> >::iterator it = sockets.begin(); it != sockets.end(); ++it) 
    {
        string localIp, localPort;
        getLocalName(it->first, localIp, localPort);

        // Optimization: Resolve local socket name only if ports match
        if (localPort == port) 
        {
            string socketV4 = getIpByHost(localIp, localPort, AF_INET);
            string socketV6 = getIpByHost(localIp, localPort, AF_INET6);

            // Check if the Target resolves to the same Identity as the Socket
            if ((!targetV4.empty() && targetV4 == socketV4) || 
                (!targetV6.empty() && targetV6 == socketV6))
            {
				INFO() << "Sharing existing socket fd=" << it->first << " for server '" << srv.serverName << "' on " << host << ":" << port;
                it->second.push_back(srv);
                
                return;
            }
        }
    }
    WARN() << "Could not find a socket to share for " << host << ":" << port << ", server '" << srv.serverName << "' will not be available.";
}

Socket::~Socket()
{
	DDEBUG("Socket") << "Socket destructor called, fd=" << fd;
	delete context;
	close(fd);
	Singleton::GetFds().erase(this);
}

