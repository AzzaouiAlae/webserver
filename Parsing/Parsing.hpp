#pragma once

#include "../Headers.hpp"

class Parsing 
{
	AST<std::string> ASTroot;
	AST<std::string>* currentNode;
	std::vector<std::string>& tokens;
	void AddServerType(std::vector<std::string>& tokens, int idx);
public:
	Parsing(std::vector<std::string>& tokens);
	AST<std::string>& GetRoot();
	void ParseConfigFile();
	
};