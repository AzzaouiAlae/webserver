#include "Repsense.hpp"

// Does one thing: initializes all pointers to NULL
Repsense::Repsense()
{
	method = NULL;
	sock = NULL;
	router = NULL;
}

// Does one thing: deletes the method object (which is allocated with new)
Repsense::~Repsense()
{
	if (method != NULL)
	{
		delete method;
		method = NULL;
	}
}

// Does one thing: stores sock/router, creates the correct method based on request type
void Repsense::Init(SocketIO *sock, Routing *router)
{
	this->sock = sock;
	this->router = router;
}

// Does one thing: delegates HandleResponse to the correct method object
bool Repsense::HandleResponse()
{
	DEBUG() << "Socket fd: " << sock->GetFd() << ", Repsense::HandleResponse start";
	if (method == NULL) 
	{
		string requestMethod = router->GetRequest().getMethod();

		if (requestMethod == "GET") {
			DEBUG() << "Socket fd: " << sock->GetFd() << ", Repsense::HandleResponse create GET method";
			method = new GET(sock, router);
		}
		else if (requestMethod == "POST") {
			DEBUG() << "Socket fd: " << sock->GetFd() << ", Repsense::HandleResponse create POST method";
			method = new Post(sock, router);
		}
		else if (requestMethod == "DELETE") {
			DEBUG() << "Socket fd: " << sock->GetFd() << ", Repsense::HandleResponse create POST method";
			// method = new DELETE(sock, router);
		}
		else
			method = new GET(sock, router); // fallback: GET handles unknown methods with 405
	}

	return method->HandleResponse();
}

// Does one thing: delegates error page handling to the current method object
void Repsense::HandelErrorPages(const string &err)
{
	if (method == NULL)
	{
		method = new GET(sock, router);
	}
	method->HandelErrorPages(err);
}