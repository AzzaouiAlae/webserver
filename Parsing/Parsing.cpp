#include "../Headers.hpp"
#include "../Singleton/Singleton.hpp"
#include "../Socket/Socket.hpp"

AST<std::string> *Parsing::currentServer;
AST<std::string> *Parsing::currentLocation;
AST<std::string> *Parsing::currentDirective;
std::vector<std::string> *Parsing::currentDirecAgs;

void Parsing::AddServer()
{
	AST<std::string> &root = Singleton::GetASTroot();

	root.AddChild("server");
	int idx = root.GetChildren().size() - 1;
	currentServer = &((root.GetChildren())[idx]);
}

void Parsing::AddLocation(AST<std::string> &server)
{
	server.AddChild("location");
	int idx = server.GetChildren().size() - 1;
	currentLocation = &((server.GetChildren())[idx]);
}

void Parsing::AddDirective(AST<std::string> &node, std::string value)
{
	node.AddChild(value);
	int idx = node.GetChildren().size() - 1;
	currentDirective = &((node.GetChildren())[idx]);
}

void Parsing::AddArg(AST<std::string> &node, std::string arg)
{
	node.AddArgument(arg);
}

void Parsing::FillConf()
{
	Config &conf = Singleton::GetConf();

	vector<AST<string> > &servers = Singleton::GetASTroot().GetChildren();
	for (int i = 0; i < (int)servers.size(); i++)
	{
		vector<AST<string> > &ch = (servers[i]).GetChildren();
		Config::Server srv;
		for (int i = 0; i < (int)ch.size(); i++)
		{
			vector<string> &args = ch[i].GetArguments();
			string val = ch[i].GetValue();
			if (val == "listen")
			{
				
				if (find(srv.listen.begin(), srv.listen.end(), args[0]) != srv.listen.end()) {
					Error::ThrowError("The same server has duplicate host and port");
				}
				srv.listen.push_back(args[0]);
				pair<string, string> p;
				parseListen(args[0], p.second, p.first);
				srv.hosts.push_back(p);
			}	
			else if (val == "error_page")
			{
				srv.errorPages[args[0]] = args[1];
			}
			else if (val == "server_name")
			{
				srv.serverName = args[0];
			}
			else if (val == "index")
			{
				for (int i = 0; i < (int)args.size(); i++)
				{
					srv.index.push_back(args[i]);
				}
			}
			else if (val == "root")
			{
				srv.root = args[0];
			}
			else if (val == "autoindex")
			{
				srv.autoindex = args[0] == "on";
			}
			else if (val == "client_max_body_size")
			{
				char *endptr;
				srv.isMaxBodySize = true;
				srv.clientMaxBodySize = std::strtoll((args[0]).c_str(), &endptr, 10);
				if (*endptr == 'k' || *endptr == 'K')
					srv.clientMaxBodySize *= 1024;
				else if (*endptr == 'm' || *endptr == 'M')
					srv.clientMaxBodySize *= 1024 * 1024;
				else if (*endptr == 'G' || *endptr == 'G')
					srv.clientMaxBodySize *= 1024 * 1024 * 1024;
			}
			else if (val == "allow_methods")
			{
				srv.allowMethodExists = true;
				for (int i = 0; i < (int)args.size(); i++)
				{
					srv.allowMethods.push_back(args[i]);
				}
			}
			else if (val == "location")
			{
				Config::Server::Location loc;
				loc.path = args[0];
				parseLocation(ch[i], loc);
				srv.Locations.push_back(loc);
			}
			
			else
			{
				Logging::Debug() << "value: " << val;
				Error::ThrowError("Some invalid value");
			}
		}
		conf.Servers.push_back(srv);
	}
	conf.listenToAllHosts();
}

void Parsing::parseLocation(AST<string> &location, Config::Server::Location &loc)
{
	vector<AST<string> > &ch = location.GetChildren();
	Utility::parseBySep(loc.parsedPath, loc.path, "/");

	for (int i = 0; i < (int)ch.size(); i++)
	{
		vector<string> &args = ch[i].GetArguments();
		string val = ch[i].GetValue();

		if (val == "index")
		{
			for (int i = 0; i < (int)args.size(); i++)
			{
				loc.index.push_back(args[i]);
			}
		}
		else if (val == "allow_methods")
		{
			loc.allowMethodExists = true;
			for (int i = 0; i < (int)args.size(); i++)
			{
				loc.allowMethods.push_back(args[i]);
			}
		}
		else if (val == "root")
		{
			loc.root = args[0];
		}
		else if (val == "cgi_pass")
		{
			loc.cgiPassExt = args[0];
			loc.cgiPassPath = args[1];
		}
		else if (val == "return")
		{
			loc.redirectionCode = args[0];
			loc.redirectionURI = args[1];
		}
		else if (val == "client_max_body_size")
		{
			char *endptr;
			loc.isMaxBodySize = true;
			loc.clientMaxBodySize = std::strtoll((args[0]).c_str(), &endptr, 10);
			if (*endptr == 'k' || *endptr == 'K')
				loc.clientMaxBodySize *= 1024;
			else if (*endptr == 'm' || *endptr == 'M')
				loc.clientMaxBodySize *= 1024 * 1024;
			else if (*endptr == 'G' || *endptr == 'G')
				loc.clientMaxBodySize *= 1024 * 1024 * 1024;
		}
		else if (val == "client_body_in_file_only")
		{
			loc.clientBodyInFileOnly = (args[0] == "on");
		}
		else
		{
			Logging::Debug() << "value: " << val;
			Error::ThrowError("Some invalid value");
		}
	}
}

void Parsing::parseListen(string str, string &port, string &host)
{
	const char *s1 = strrchr(str.c_str(), ':');
	bool isHost = false;

	if (s1 != NULL)
	{
		host = str.c_str();
		host = host.substr(0, (s1 - str.c_str()));
		port = s1 + 1;
		return;
	}
	for(int i = 0; i < (int)str.length(); i++)
	{
		if(isdigit(str[i]) == false)
		{
			isHost = true;
			break;
		}
	}
	if (isHost) {
		host = str;
		port = "80";
	}
	else
		port = str;
}

