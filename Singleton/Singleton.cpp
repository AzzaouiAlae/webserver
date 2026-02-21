#include "Singleton.hpp"

AST<string>& Singleton::GetASTroot()
{
    static AST<string> ASTroot("ASTroot");

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

set<AFd *> &Singleton::GetFds()
{
	static set<AFd *> fds;

	return fds;
} 

map<int, vector<AST<string> > > &Singleton::GetServers()
{
	static map<int, vector<AST<string> > > servers;

	return servers;
}

DeprecatedConfig &Singleton::GetConf()
{
	static DeprecatedConfig conf;

	return conf;
}

map<int, vector<DeprecatedConfig::DeprecatedServer> > &Singleton::GetVirtualServers()
{
	static map<int, vector<DeprecatedConfig::DeprecatedServer> > virtualServers;

	return virtualServers;
}

