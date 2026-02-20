#pragma once
#include "../Headers.hpp"
#include "../AMethod/AMethod.hpp"

class Delete: public AMethod
{
	void deleteFile();
	Config::Server::Location *loc;
public:
	bool HandleResponse();
	Delete(SocketIO *sock, Routing *router);
};