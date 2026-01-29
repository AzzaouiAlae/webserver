#pragma once
#include "../Headers.hpp"

class Singleton 
{

public:
	static AST<std::string>& GetASTroot();
	static vector<int>& GetSockets();
};