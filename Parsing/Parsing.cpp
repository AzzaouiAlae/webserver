# include "../Headers.hpp"

AST<std::string>* Parsing::currentServer;
AST<std::string>* Parsing::currentLocation;
AST<std::string>* Parsing::currentDirective;
std::vector<std::string>* Parsing::currentDirecAgs;

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

void find(std::string str, int lvl)
{
	(void)str;
	(void)lvl;
}

std::vector<std::string> Parsing::GetDirecAgs(AST<std::string>& node, std::string& direcName)
{
	(void)direcName;
	std::vector<std::string> strs;
	currentDirecAgs = &strs;
	node.InorderTraversal(find);
	return strs;
}
// std::vector<AST<std::string> > Parsing::GetNode(AST<std::string>& node, std::string& nodeName)
// {
// 	(void)node;
// 	(void)nodeName;

	
// }
bool isServer(vector<string>& strs, string& host, string& port)
{
	if ((int)strs.size() == 2)
	{
		if (strs[0] == host && port == strs[1])
		{
			return true;
		}
	}
	else if ((int)strs.size() == 1)
	{
		if (port == strs[0])
		{
			return true;
		}
	}
	return false;
}

AST<std::string>& Parsing::GetServerByHost(string host, string port)
{
	AST<std::string>& ASTroot = Singleton::GetASTroot();

	std::vector<AST<string> >& servers = ASTroot.GetChildren();
	for(int i = 0; i < (int)servers.size(); i++)
	{
		if (servers[i].GetValue() == "listen")
		{
			if (isServer(servers[i].GetArguments(), host, port))
				return servers[i];
		}
	}
	Error::ThrowError("Error: GetServerByHost\n");
	return ASTroot;
}



AST<std::string>& Parsing::GetLocationByPath(AST<std::string>& server, string& path)
{
	int len = 0, j;
	AST<std::string> *loc = NULL, *location = NULL;
	string str;

	for(int i = 0; i < (int)server.GetChildren().size(); i++)
	{
		if ((server.GetChildren())[i].GetValue() == "location")
		{
			loc = &(server.GetChildren())[i];
			str = (loc->GetArguments())[0];
			for(j = 0; j < (int)str.size() && str[j] == path[j]; j++);
			if (j > len)
			{
				location = loc;
				len = j;
			}
		}
	}
	if (location == NULL)
		Error::ThrowError("Error: GetLocationByPath\n");
	return *location;
}

string Parsing::GetRoot(AST<std::string>& node)
{
	vector<AST<std::string> >& ch = node.GetChildren();
	for (int i = 0; i < (int)ch.size(); i++)
	{
		if (ch[i].GetValue() == "root")
		{
			return (ch[i].GetArguments())[0];
		}
	}
	return "";
}


// server h;