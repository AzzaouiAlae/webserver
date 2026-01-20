#pragma once
#include <iostream>
#include "../AbstractSyntaxTree/AST.hpp"

class TokensTypes
{
public:
	static NodeType GetNodeType(std::vector<std::string>& tokens, int idx);
};