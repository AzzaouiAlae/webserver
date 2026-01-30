#pragma once

#include "../Headers.hpp"

class Parsing 
{
public:
	static AST<std::string>* currentServer;
	static AST<std::string>* currentLocation;
	static AST<std::string>* currentDirective;
	Parsing();
	static AST<std::string>& AddServer();
	static AST<std::string>& AddLocation(AST<std::string>& server);
	static AST<std::string>& AddDirective(AST<std::string>& node, std::string value);
	static void AddArg(AST<std::string>& node, std::string arg);
}; 
