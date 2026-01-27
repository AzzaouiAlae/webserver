#pragma once

#include "../Headers.hpp"

class Parsing 
{
public:
	static AST<std::string>* currentServer;
	static AST<std::string>* currentLocation;
	static AST<std::string>* currentDirective;
	Parsing();
	static void AddServer();
	static void AddLocation(AST<std::string>& server);
	static void AddDirective(AST<std::string>& node, std::string value);
	static void AddArg(AST<std::string>& node, std::string arg);
}; 
