#include "Response.hpp"

Repsense::Repsense()
{
	method = NULL;
	sock = NULL;
	router = NULL;
}

Repsense::~Repsense()
{
	if (method != NULL)
	{
		delete method;
		method = NULL;
	}
}

void Repsense::Init(SocketIO *sock, Routing *router)
{
	this->sock = sock;
	this->router = router;
}

bool Repsense::HandleResponse()
{
	DEBUG("Repsense") << "Socket fd: " << sock->GetFd() << ", Repsense::HandleResponse start";
	if (method == NULL) 
	{
		string requestMethod = router->GetRequest().getMethod();

		if (requestMethod == "GET") {
			DEBUG("Repsense") << "Socket fd: " << sock->GetFd() << ", creating GET handler";
			method = new GET(sock, router);
		}
		else if (requestMethod == "POST") {
			DEBUG("Repsense") << "Socket fd: " << sock->GetFd() << ", creating POST handler";
			method = new Post(sock, router);
		}
		else if (requestMethod == "DELETE") {
			DEBUG("Repsense") << "Socket fd: " << sock->GetFd() << ", creating DELETE handler";
			method = new Delete(sock, router);
		}
		else
			method = new GET(sock, router);
	}

	return method->HandleResponse();
}

void Repsense::HandelErrorPages(const string &err)
{
	if (method == NULL)
	{
		method = new GET(sock, router);
	}
	method->HandelErrorPages(err);
}
