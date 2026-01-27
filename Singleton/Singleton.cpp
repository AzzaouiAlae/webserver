#include "Singleton.hpp"

Singleton::Map& Singleton::GetEnv()
{
    static Map env;

    return env;
}


AST<std::string>& Singleton::GetASTroot()
{
    static AST<std::string> ASTroot("ASTroot");

    return ASTroot;
}