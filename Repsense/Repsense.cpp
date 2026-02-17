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

	if (method != NULL)
		return;

	string requestMethod = router->GetRequest().getMethod();

	if (requestMethod == "GET")
		method = new GET();
	else if (requestMethod == "POST") {
		// method = new POST();
	}
	else if (requestMethod == "DELETE") {
		// method = new DELETE();
	}
	else
		method = new GET(); // fallback: GET handles unknown methods with 405

	method->Init(sock, router);
}

// Does one thing: delegates HandleResponse to the correct method object
bool Repsense::HandleResponse()
{
	return method->HandleResponse();
}

// Does one thing: delegates error page handling to the current method object
void Repsense::HandelErrorPages(const string &err)
{
	if (method == NULL)
	{
		method = new GET();
		method->Init(sock, router);
	}
	method->HandelErrorPages(err);
}