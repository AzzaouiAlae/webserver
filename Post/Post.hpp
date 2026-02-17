#pragma once
#include "../Headers.hpp"
#include "../AMethod/AMethod.hpp"

class Post: public AMethod
{
public:
	Post(SocketIO *sock, Routing *router);
	bool HandleResponse();
};
