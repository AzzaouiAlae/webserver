#pragma once
#include "../Headers.hpp"
#include <sys/sendfile.h>

class Routing 
{
	Path path;
	string strPath;
	Request request;
	bool RequestComplete;
public:
	AST<string> *srv;
	Request &GetRequest();
	Path &GetPath();
	Routing();
	bool isRequestComplete();
	void SetRequestComplete();
	string CreatePath(vector<AST<string> > *servers);
};

