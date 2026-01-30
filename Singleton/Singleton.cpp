#include "Singleton.hpp"

AST<std::string>& Singleton::GetASTroot()
{
    static AST<std::string> ASTroot("ASTroot");

    return ASTroot;
}

