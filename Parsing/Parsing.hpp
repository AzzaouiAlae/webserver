#pragma once
#include "../AbstractSyntaxTree/AST.hpp"

class Parsing 
{
	AST<std::string> root;

public:
	Parsing(std::vector<std::string>);
};