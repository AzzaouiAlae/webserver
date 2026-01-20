#pragma once

#include "../AbstractSyntaxTree/AST.hpp"

class Parsing 
{
	AST<std::string> root;
	std::vector<std::string>& tokens;

public:
	Parsing(std::vector<std::string>& tokens);
	AST<std::string>& GetRoot();
	void ParseConfigFile();
};