#pragma once
#include "../Headers.hpp"
#include <sys/sendfile.h>

class Routing 
{
	bool sended;
	bool isReadyToSend;
	bool created;
	string path;
	request* req;
	string header;
	int pathFD;
public:
	Routing();
	bool isResponseSend();
	bool isCreated();
	void CreatePath();
	int SendResponse(int sock, int size = 1024);
	void CreateResponse(request& req);
	void CreadHeader(string type, int size);
	bool ReadyToSend();
};

