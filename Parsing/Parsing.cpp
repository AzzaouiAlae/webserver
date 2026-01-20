#include "Parsing.hpp"
#include "../TokensTypes/TokensTypes.hpp"

Parsing::Parsing(std::vector<std::string>& tokens): ASTroot("ASTroot"), tokens(tokens)
{
	currentNode = NULL;
}

AST<std::string>& Parsing::GetRoot()
{
	return ASTroot;
}

void Parsing::AddServerType(std::vector<std::string>& tokens, int idx)
{
	currentNode->AddChild(tokens[idx]);
}

void Parsing::ParseConfigFile()
{
	NodeType type;
	currentNode = &ASTroot;
	for(int i = 0; i < (int)tokens.size(); i++)
	{
		type = TokensTypes::GetNodeType(tokens, i);
		switch (type)
		{
		case server:
			AddServerType(tokens, i);
			break;
		case listen:
			break;
		case root:
			break;
		case index:
			break;
		case server_name:
			break;
		case location:
			break;
		}
		AST<std::string> node("");
	}
}