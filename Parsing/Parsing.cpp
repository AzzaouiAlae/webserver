# include "../Headers.hpp"
#include "../Singleton/Singleton.hpp"

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

void Parsing::GetHosts(vector<string > &hosts, vector<AST<string> > &srvChildren)
{
	for(int i = 0; i < (int)srvChildren.size(); i++)
	{
		if (srvChildren[i].GetValue() == "listen" && srvChildren[i].GetArguments().size() > 0)
		{
			string s = (srvChildren[i].GetArguments())[0];
			hosts.push_back(s);
		}
	}
}

bool Parsing::IsduplicatedListen(string &host, vector<AST<string> > &srvChildren)
{
	for(int i = 0; i < (int)srvChildren.size(); i++)
	{
		int size = srvChildren[i].GetArguments().size();
		string val = srvChildren[i].GetValue();
		if (val == "listen" && size > 0)
		{
			string s = (srvChildren[i].GetArguments())[0];
			if (s == host)
				return true;
		}
	}
	return false;
}

string Parsing::GetServerName(AST<string>& server)
{
	string name = "";
	vector<AST<string> > &children = server.GetChildren();
	for(int i = 0; i < (int)children.size(); i++)
	{
		AST<string> &childe = children[i];
		if (childe.GetValue() != "server_name")
			continue;
		if (childe.GetArguments().size() == 0)
			continue;
		name = (childe.GetArguments())[0];
		break;
	}
	return name;
}

AST<string>* Parsing::GetServerByName(const string &srvName, int &start, int end)
{
	vector<AST<string> > &servers = Singleton::GetASTroot().GetChildren();
	string sName;
	for( ; start < (int)servers.size() - end; start++)
	{
		sName = GetServerName(servers[start]);
		if (sName == srvName)
			return &(servers[start]);
	}
	return NULL;
}

bool Parsing::IsDuplicatedServer(const string &srvName, const string &host)
{
	int start = 0;
	while(start < (int)Singleton::GetASTroot().GetChildren().size() - 1)
	{
		AST<string>* server = GetServerByName(srvName, start, 1);
		if (server == NULL)
			return false;
		if (IsduplicatedListen((string &)host, server->GetChildren()))
			return true;
		start++;
	}
	return false;
}

AST<string>* Parsing::GetServerByName(const string &srvName, int &start, vector<AST<string> > &srvs)
{
	string sName;

	for( ; start < (int)srvs.size(); start++)
	{
		sName = GetServerName(srvs[start]);
		if (sName == srvName) 
			return &(srvs[start]);
	}
	return NULL;
}

bool Parsing::IsDuplicatedServer(const string &srvName, const string &host, vector<AST<string> > &srvs)
{
	int start = 0;
	while(start < (int)srvs.size())
	{
		AST<string>* server = GetServerByName(srvName, start, srvs);
		if (server == NULL)
			return false;
		if (IsduplicatedListen((string &)host, server->GetChildren()))
			return true;
		start++;
	}
	return false;
}

AST<string>* Parsing::GetServerByHost(vector<AST<string> > &srvs, const string &host)
{
	vector<string> hosts;
	for(int i = 0; i < (int)srvs.size(); i++)
	{
		hosts.clear();
		GetHosts(hosts, srvs[i].GetChildren());
		if (host.find(host) != host.npos)
			return &(srvs[i]);
	}
	return &(srvs[0]);
}