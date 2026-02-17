#pragma once
#include "../Headers.hpp"
#include "../AMethod/AMethod.hpp"

class Post: public AMethod
{
	bool readyToUpload;
	size_t contentBodySize;
	size_t uploadedSize;

public:
	Post(SocketIO *sock, Routing *router);
	bool HandleResponse();
	void PostMethod();
};
