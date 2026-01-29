#include "Routing.hpp"

void Routing::CreatePath()
{
	AST<std::string> *loc;
	AST<std::string> *srv;
	try {
		srv = &Parsing::GetServerByHost(req->host, req->port);
		loc = &Parsing::GetLocationByPath(*srv, req->path);
	} catch (exception &e) {
	
	}
	
	(void)srv;
	(void)loc;
}

void Routing::SendResponse()
{

}

void Routing::CreateResponse(request& req)
{
	this->req = &req;
	CreatePath();
	if (req.method == "GET")
	{
		
	}
}

bool Routing::isResponseSend()
{
	return sended;
}

Routing::Routing()
{
	sended = false;
}

bool Routing::isCreated()
{
	return created;
}