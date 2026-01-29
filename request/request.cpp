#include "request.hpp"

void request::AddBuffer(char *buff)
{
	string s = buff;
	size_t idx = 0;
	if (req.find("\n\r") == string::npos)
	{
		idx = s.find("\n\r");
		if (idx != string::npos)
			req += s.substr(0, idx);
		idx++;
	}
	body = s.substr(idx);
}

void request::ParseRequest()
{
	method = strtok((char *)req.c_str(), " ");
	path = strtok(NULL, " ");
	strtok(NULL, "\n");
	strtok(NULL, " ");
	host = strtok(NULL, ":");
	port = strtok(NULL, "\n");
}

bool request::isComplete()
{
	if (req.find("\n\r") == string::npos)
	{
		ParseRequest();
		return true;
	}
	return false;
}
