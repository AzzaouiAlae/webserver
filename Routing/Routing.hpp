#pragma once
#include "../Headers.hpp"

class Routing 
{
	bool sended;
	bool created;
	string path;
	Request* req;
public:
	Routing();
	bool isResponseSend();
	bool isCreated();
	void CreatePath();
	void SendResponse();
	void CreateResponse(Request& req);
};

