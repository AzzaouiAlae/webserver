#pragma once
#include "../Headers.hpp"
#include <sys/sendfile.h>

class Routing 
{
	string header;
	Path path;
	Request request;
	bool RequestComplete;
public:
	Request &GetRequest();
	Path &GetPath();
	Routing();
	bool isRequestComplete();
	void SetRequestComplete();
	void CreatePath();
};

