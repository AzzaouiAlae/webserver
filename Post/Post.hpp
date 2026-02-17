#pragma once
#include "../Headers.hpp"
#include "../AMethod/AMethod.hpp"

class Post: public AMethod
{
	bool readyToUpload;
	size_t contentBodySize;
	size_t uploadedSize;
	void uploadFileToDisk();
	void createPostRespence();
	void PostMethod();

public:
	Post(SocketIO *sock, Routing *router);
	~Post();
	bool HandleResponse();
};
