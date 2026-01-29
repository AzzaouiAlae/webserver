#pragma once
#include "../Headers.hpp"

class Routing 
{
	bool sended;
	bool created;
	string path;
	request* req;
public:
	Routing();
	bool isResponseSend();
	bool isCreated();
	void CreatePath();
	void SendResponse();
	void CreateResponse(request& req);
};

