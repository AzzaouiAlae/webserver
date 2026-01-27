# include "../Headers.hpp"

AST<std::string>* Parsing::currentServer;
AST<std::string>* Parsing::currentLocation;
AST<std::string>* Parsing::currentDirective;

void Parsing::AddServer()
{
	AST<std::string>& root = Singleton::GetASTroot();

	root.AddChild("server");
	int idx = root.GetChildren().size() - 1;
	currentServer = &((root.GetChildren())[idx]);
}
void Parsing::AddLocation(AST<std::string>& server)
{
	server.AddChild("location");
	int idx = server.GetChildren().size() - 1;
	currentLocation = &((server.GetChildren())[idx]);
}
void Parsing::AddDirective(AST<std::string>& node, std::string value)
{
	node.AddChild(value);
	int idx = node.GetChildren().size() - 1;
	currentDirective = &((node.GetChildren())[idx]);
}
void Parsing::AddArg(AST<std::string>& node, std::string arg)
{
	node.AddArgument(arg);
}
