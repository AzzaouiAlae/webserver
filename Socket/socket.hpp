#pragma once
#include "../Headers.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

enum SockStatus 
{
	eSockCreated,
	eGetaddrinfo,
	eListen,
	eCanNotBindAnySocket
};

class Socket
{
	static bool isValidePort(string &port);
	static bool BindAddrInfo(addrinfo *rp, int *sock, int *optVal);
	static bool CreateAddrInfo(string &host, string& port, addrinfo *hints, addrinfo **result);
public:
	Socket();
	static SockStatus AddSocket(string host, string port);
};