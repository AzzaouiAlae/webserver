#include "Routing.hpp"

Routing::Routing()
{
	RequestComplete = false;
}
Request &Routing::GetRequest()
{
	return request;
}

bool Routing::isRequestComplete()
{
	return RequestComplete;
}

void Routing::SetRequestComplete()
{
	RequestComplete = true;
}

Path &Routing::GetPath()
{
	return path;
}

void Routing::CreatePath()
{
	
}