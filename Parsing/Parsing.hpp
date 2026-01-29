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
	static std::vector<std::string> GetDirecAgs(AST<std::string>& node, std::string& direcName);
	static std::vector<AST<std::string> > GetNode(AST<std::string>& node, std::string& nodeName);
	static void AddServer();
	static void AddLocation(AST<std::string>& server);
	static void AddDirective(AST<std::string>& node, std::string value);
	static void AddArg(AST<std::string>& node, std::string arg);
	static AST<std::string>& GetServerByHost(string host, string port);
	static AST<std::string>& GetLocationByPath(AST<std::string>& server, string& path);
	static string GetRoot(AST<std::string>& node);
}; 
