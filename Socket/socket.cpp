#include "../Headers.hpp"

bool SocketIO::isValidePort(string &port)
{
	if (isdigit(port[0]))
	{
		if (port.length() > 5)
			return false;
		for (int i = 0; port[i]; i++)
		{
			if (!isdigit(port[i]))
				return false;
		}
		int p = atoi(port.c_str());
		if (p >= 65536)
			return false;
	}
	return true;
}

bool SocketIO::CreateAddrInfo(string &host, string &port, addrinfo *hints, addrinfo **result)
{
	Utility::ltrim(port, Utility::isNotZero);
	Utility::trim(host, Utility::isNotSquareBracket);

	if (isValidePort(port) == false)
		return false;

	memset(hints, 0, sizeof(*hints));
	hints->ai_socktype = SOCK_STREAM;
	hints->ai_family = AF_UNSPEC;
	hints->ai_flags = AI_PASSIVE;

	if (getaddrinfo(host.c_str(), port.c_str(), hints, result) != 0)
	{
		return false;
	}
	return true;
}

bool SocketIO::BindAddrInfo(addrinfo *rp, int *sock, int *optVal)
{
	*sock = socket(rp->ai_family, rp->ai_socktype | SOCK_CLOEXEC | SOCK_NONBLOCK, rp->ai_protocol);
	if (*sock == -1)
		return false;
	if (rp->ai_family == AF_INET6)
	{
		if (setsockopt(*sock, IPPROTO_IPV6, IPV6_V6ONLY, &optVal, sizeof(optVal)) == -1)
			return false;
	}
	else if (rp->ai_family == AF_INET)
	{
		if (setsockopt(*sock, SOL_SOCKET, SO_REUSEADDR,
					optVal, sizeof(*optVal)) == -1)
			return false;
	}
	if (bind(*sock, rp->ai_addr, rp->ai_addrlen) == -1)
	{
		return false;
	}
	return true;
}

SockStatus SocketIO::AddSocket(string host, string port)
{
	vector<int> &sockets = Singleton::GetSockets();
	int sock, optVal = 1;
	addrinfo hints, *result, *rp;
	int backLog = 10;

	if (host == "")
		host = "0.0.0.0";

	if (CreateAddrInfo(host,
					port, &hints, &result) == false)
		return eGetaddrinfo;

	for (rp = result; rp; rp = rp->ai_next)
	{
		if (BindAddrInfo(rp, &sock, &optVal) == true)
		{
			if (listen(sock, backLog) == -1)
				continue;

			sockets.push_back(sock);
			freeaddrinfo(result);
			return eSockCreated;
		}
	}
	freeaddrinfo(result);
	return eCanNotBindAnySocket;
}
