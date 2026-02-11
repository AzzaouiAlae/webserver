#pragma once
#include "../Headers.hpp"

class Parsing 
{
	static std::vector<std::string>* currentDirecAgs;
public:
	static AST<std::string>* currentServer;
	static AST<std::string>* currentLocation;
	static AST<std::string>* currentDirective;
	Parsing();
	static void AddServer();
	static void AddLocation(AST<std::string>& server);
	static void AddDirective(AST<std::string>& node, std::string value);
	static void AddArg(AST<std::string>& node, std::string arg);
	static bool IsduplicatedListen(string &host, vector<AST<string> > &srvChildren);

	static void GetHosts(vector<string > &hosts, vector<AST<string> > &srvChildren);
	static string GetServerName(AST<string>& server);
	static AST<string>* GetServerByName(const string &srvName, int &start);
	static bool IsDuplicatedServer(const string &srvName, const string &host);
	static bool IsDuplicatedServer(const string &srvName, const string &host, vector<AST<string> > &srvs);
	static AST<string>* GetServerByName(const string &srvName, int &start, vector<AST<string> > &srvs);
	static AST<string>* GetServerByHost(vector<AST<string> > &srvs, const string &host);
}; 
