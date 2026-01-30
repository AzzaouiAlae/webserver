#include "Singleton.hpp"

AST<std::string>& Singleton::GetASTroot()
{
    static AST<std::string> ASTroot("ASTroot");

    return ASTroot;
}

vector<int>& Singleton::GetSockets()
{
	static vector<int> sockts;

	return sockts;
}


map<string, string> &Singleton::GetMime()
{
	static map<string, string> mime;

	return mime;
}