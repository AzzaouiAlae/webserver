#include "Parsing.hpp"
#include "../TokensTypes/TokensTypes.hpp"


Parsing::Parsing(std::vector<std::string>& tokens): root("ASTroot"), tokens(tokens)
{}

AST<std::string>& Parsing::GetRoot()
{
	return root;
}

void Parsing::ParseConfigFile()
{
	NodeType type;
	for(int i = 0; i < (int)tokens.size(); i++)
	{
		type = TokensTypes::GetNodeType(tokens, i);
		AST<std::string> node("");
	}
	
	(void)type;
}