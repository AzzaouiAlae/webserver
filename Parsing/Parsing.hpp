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
	static AST<std::string>& AddServer();
	static std::vector<std::string> GetDirecAgs(AST<std::string>& node, std::string& direcName);
	static std::vector<AST<std::string> > GetNode(AST<std::string>& node, std::string& nodeName);
	static AST<std::string>& AddLocation(AST<std::string>& server);
	static AST<std::string>& AddDirective(AST<std::string>& node, std::string value);
	static void AddArg(AST<std::string>& node, std::string arg);
}; 
