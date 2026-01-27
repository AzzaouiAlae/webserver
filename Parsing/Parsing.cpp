#include "Parsing.hpp"

AST<std::string>* Parsing::currentServer;
AST<std::string>* Parsing::currentLocation;
AST<std::string>* Parsing::currentDirective;

AST<std::string>& Parsing::AddServer()
{
	AST<std::string>& root = Singleton::GetASTroot();

	root.AddChild("server");
	int idx = root.GetChildren().size() - 1;
	currentServer = &((root.GetChildren())[idx]);
	return *currentServer;
}
AST<std::string>& Parsing::AddLocation(AST<std::string>& server)
{
	server.AddChild("location");
	int idx = server.GetChildren().size() - 1;
	currentLocation = &((server.GetChildren())[idx]);
	return *currentLocation;
}
AST<std::string>& Parsing::AddDirective(AST<std::string>& node, std::string value)
{
	node.AddChild(value);
	int idx = node.GetChildren().size() - 1;
	currentDirective = &((node.GetChildren())[idx]);
	return *currentDirective;
}
void Parsing::AddArg(AST<std::string>& node, std::string arg)
{
	node.AddArgument(arg);
}

void find(std::string str, int lvl)
{

}

std::vector<std::string> Parsing::GetDirecAgs(AST<std::string>& node, std::string& direcName)
{
	std::vector<std::string> strs;
	currentDirecAgs = &strs;
	node.InorderTraversal(find);
	return strs;
}
std::vector<AST<std::string>> Parsing::GetNode(AST<std::string>& node, std::string& nodeName)
{

}