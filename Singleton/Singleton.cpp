#include "Singleton.hpp"

AST<std::string>& Singleton::GetASTroot()
{
    static AST<std::string> ASTroot("ASTroot");

    return ASTroot;
}

vector<int>& Singleton::GetSockets()
{
	static vector<int> sockts;

	return sockts;
}

map<string, string> &Singleton::GetMime()
{
	static map<string, string> mime;

	return mime;
}

vector<AFd *> &Singleton::GetFds()
{
	static vector<AFd *> fds;

	return fds;
} 

map<int, vector<AST<string> > > &Singleton::GetServers()
{
	static map<int, vector<AST<string> > > servers;

	return servers;
}