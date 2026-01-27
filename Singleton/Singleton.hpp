#pragma once
#include "../Headers.hpp"

class Singleton 
{
    typedef std::map<std::string, std::string> Map;
public:
    static Map& GetEnv();
	static AST<std::string>& GetASTroot();
	
};