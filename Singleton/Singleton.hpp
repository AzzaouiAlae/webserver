#pragma once
#include "../Headers.hpp"

class Singleton 
{

public:
	static AST<string>& GetASTroot();
	static vector<int>& GetSockets();
	static map<string, string>& GetMime();
	static set<AFd* > &GetFds();
	static map<int, vector<AST<string> > > &GetServers();
};