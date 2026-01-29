#pragma once
#include "../Headers.hpp"

class request
{
	string req;
	string body;
public:
	string method;
	string path;
	string host;
	string port;
	map<string, string> env;
	void AddBuffer(char *buff);
	void ParseRequest();
	bool isComplete();
};