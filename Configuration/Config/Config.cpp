#include "Config.hpp"
#include "Socket.hpp"

Config::Server::Server()
{
	autoindex = -1;
	clientMaxBodySize = 0;
	allowMethodExists = false;
	isMaxBodySize = false;
	autoindex = -1;
	keepAliveTimeout = 60 * USEC;
	clientReadTimeout = 30 * USEC;
	cgiTimeout = 10 * USEC;
	clientMaxBodySize = UINT64_MAX;
}

Config::Server::Location::Location()
{
	allowMethodExists = false;
	clientMaxBodySize = 0;
	isMaxBodySize = false;
	deleteFiles = false;
	chunkedSend = true;
	autoindex = -1;
	clientBodyInFileOnly = false;
	clientMaxBodySize = UINT64_MAX;
}

bool Config::IsDuplicatedServer(int currServerIdx, const string &val, const string &srvName)
{
	for (int i = 0; i < currServerIdx; i++)
	{
		Server srv = Servers[i];
		if (srvName != srv.serverName)
			continue;
		for (int j = 0; j < (int)srv.listen.size(); j++)
		{
			if (val == srv.listen[j])
			{
				return true;
			}
		}
	}
	return false;
}

void Config::listenToAllHosts()
{
	INFO() << "Setting up listening sockets for " << Servers.size() << " server(s).";
	for (int i = 0; i < (int)Servers.size(); i++)
	{
		Server srv = Servers[i];
		for (int j = 0; j < (int)srv.listen.size(); j++)
		{
			if (IsDuplicatedServer(i, srv.listen[j], srv.serverName))
			{
				DEBUG("Config") << "Skipping duplicate server '" << srv.serverName << "' on " << srv.listen[j];
				continue;
			}
			DEBUG("Config") << "Adding socket for server '" << srv.serverName << "' on " << srv.listen[j];
			Socket::AddSocketNew(srv.hosts[j].first, srv.hosts[j].second, srv);
		}
	}
}

Config::Server &Config::GetServerName(vector<Config::Server> &srvs, const string &val)
{
	for (int i = 0; i < (int)srvs.size(); i++)
	{
		if (val == srvs[i].serverName)
			return srvs[i];
	}
	for (int i = 0; i < (int)srvs.size(); i++)
	{
		vector<string> &hosts = srvs[i].listen;
		for (int j = 0; j < (int)hosts.size(); j++)
		{
			if (val == hosts[j])
				return srvs[i];
		}
	}
	return srvs[0];
}

size_t Config::GetMaxBodySize(vector<Config::Server> &srvs)
{
	size_t size = 0;
	for (int i = 0; i < (int)srvs.size(); i++)
	{
		if (size < srvs[i].clientMaxBodySize)
		{
			size = srvs[i].clientMaxBodySize;
		}
		vector<Config::Server::Location> &loc = srvs[i].Locations;
		for (int j = 0; j < (int)loc.size(); j++)
		{
			if (size < loc[j].clientMaxBodySize)
			{
				size = loc[j].clientMaxBodySize;
			}
		}
	}
	return size;
}

int Config::GetMaxClientReadTimeout(vector<Config::Server> &srvs)
{
	int maxTimeout = 0;
	for (int i = 0; i < (int)srvs.size(); i++)
	{
		if (srvs[i].clientReadTimeout > maxTimeout)
			maxTimeout = srvs[i].clientReadTimeout;
	}
	if (maxTimeout == 0)
		maxTimeout = 30;
	return maxTimeout;
}

string Config::GetErrorPath(Config::Server &srv, const string &code)
{
	map<string, string>::iterator it = srv.errorPages.find(code);
	if (it != srv.errorPages.end())
	{
		return it->second;
	}
	return "";
}

bool Config::isPrefixMatch(const vector<string> &locPath, const vector<string> &reqPath)
{
	if (locPath.size() > reqPath.size())
		return false;
	for (size_t j = 0; j < locPath.size(); ++j)
	{
		if (locPath[j] != reqPath[j])
			return false;
	}
	return true;
}

bool Config::isAllowedMethod(const vector<string> &allowMethods, const string &method)
{
	for (size_t i = 0; i < allowMethods.size(); ++i)
	{
		if (allowMethods[i] == method)
			return true;
	}
	return false;
}

int Config::findBestCgiMatch(Config::Server &srv, const vector<string> &reqPath, string &scriptPath, string &pathInfo)
{
	int bestIndex = -1;
	size_t maxLength = 0;
	string script;
	size_t idx = -1;
	size_t i;
	for (i = 0; i < reqPath.size(); i++)
	{
		string pathExt = Utility::GetFileExtension(reqPath[i]);
		if (i != 0)
			script += "/" + reqPath[i];
		else
			script = reqPath[i];
		if (pathExt.empty())
			continue;
		pathExt = '.' + pathExt;
		for (size_t j = 0; j < srv.Locations.size(); ++j)
		{
			const Config::Server::Location &loc = srv.Locations[j];
			if (loc.cgiPassExt.empty())
				continue;
			if (!isPrefixMatch(loc.parsedPath, reqPath))
				continue;
			if (loc.cgiPassExt != pathExt)
				continue;
			if (loc.parsedPath.size() > maxLength || bestIndex == -1)
			{
				maxLength = loc.parsedPath.size();
				bestIndex = (int)j;
				scriptPath = script;
				idx = i;
			}
		}
		if (bestIndex == -1)
			continue;
		break;
	}
	if (bestIndex != -1)
	{
		for (size_t k = idx + 1; k < reqPath.size(); k++)
		{
			if (k != idx + 1)
				pathInfo += "/" + reqPath[k];
			else
				pathInfo = reqPath[k];
		}
	}
	return bestIndex;
}

int Config::findBestStaticMatch(Config::Server &srv, const vector<string> &reqPath)
{
	int bestIndex = -1;
	size_t maxLength = 0;

	for (size_t i = 0; i < srv.Locations.size(); ++i)
	{
		const Config::Server::Location &loc = srv.Locations[i];
		if (!loc.cgiPassExt.empty())
			continue;
		if (!isPrefixMatch(loc.parsedPath, reqPath))
			continue;
		if (loc.parsedPath.size() > maxLength || bestIndex == -1)
		{
			maxLength = loc.parsedPath.size();
			bestIndex = (int)i;
		}
	}
	return bestIndex;
}

void makeCgiArgs(const Config::Server::Location &loc, string &scriptPath, string &pathInfo, const vector<string> &reqPath)
{
	string script;
	if (!loc.cgiPassExt.empty())
	{
		for (size_t i = 0; i < reqPath.size(); i++)
		{
			string pathExt = Utility::GetFileExtension(reqPath[i]);
			if (i != 0)
				script += "/" + reqPath[i];
			else
				script = reqPath[i];
			if (pathExt.empty())
				continue;
			pathExt = '.' + pathExt;
			if (pathExt != loc.cgiPassExt)
				continue;
			scriptPath = script;
			for (size_t k = i + 1; k < reqPath.size(); k++)
			{
				if (k != i + 1)
					pathInfo += "/" + reqPath[k];
				else
					pathInfo = reqPath[k];
			}
			return;
		}
	}
}

int Config::findMethodRedirects(const Config::Server::Location &loc,
								const string &method, Config::Server &srv, int bestLocIndex,
								string &scriptPath, string &pathInfo, const vector<string> &reqPath)
{
	string path;
	if (loc.methodRedirects.empty())
		return bestLocIndex;
	map<string, string>::const_iterator it = loc.methodRedirects.find(method);
	if (it != loc.methodRedirects.end())
		path = it->second;
	else
		return bestLocIndex;
	for (size_t i = 0; i < srv.Locations.size(); ++i)
	{
		if (srv.Locations[i].path == path)
		{
			const Config::Server::Location &loc = srv.Locations[i];
			makeCgiArgs(loc, scriptPath, pathInfo, reqPath);
			return (int)i;
		}
	}
	return bestLocIndex;
}

int Config::GetLocationIndex(Config::Server &srv, const string &path, string &scriptPath, string &pathInfo, const string &method)
{
	vector<string> reqPath;
	Utility::parseBySep(reqPath, path, "/");

	int bestCgiIndex = findBestCgiMatch(srv, reqPath, scriptPath, pathInfo);
	if (bestCgiIndex != -1)
	{
		DDEBUG("Config") << "Location match for '" << path << "': CGI location index " << bestCgiIndex;
		return findMethodRedirects(srv.Locations[bestCgiIndex], method, srv, bestCgiIndex, scriptPath, pathInfo, reqPath);
	}

	int bestStaticIndex = findBestStaticMatch(srv, reqPath);
	DDEBUG("Config") << "Location match for '" << path << "': static location index " << bestStaticIndex;
	if (bestStaticIndex == -1)
		return -1;
	return findMethodRedirects(srv.Locations[bestStaticIndex], method, srv, bestStaticIndex, scriptPath, pathInfo, reqPath);
}

void Config::writeIndexFile()
{
	StaticFile *staticFile = StaticFile::GetFileByName("index");
	if (staticFile == NULL)
		return;
	int fd = open("EngineX/www/index.htm", O_WRONLY | O_CREAT, 0644);
	if (fd == -1)
		return;
	write(fd, staticFile->GetData(), staticFile->GetSize());
	close(fd);
}

void Config::createDir()
{
	if (access("EngineX", F_OK) == -1)
	{
		mkdir("EngineX", 0755);
	}
	if (access("EngineX/www", F_OK) == -1)
	{
		mkdir("EngineX/www", 0755);
	}
}

void Config::FillConf()
{
	DEBUG("Parsing") << "Starting to fill Config structure from AST.";
	Config &conf = Singleton::GetConf();

	vector<AST<string> > &servers =
		Singleton::GetASTroot().GetChildren();

	for (int i = 0; i < (int)servers.size(); i++)
	{
		DDEBUG("Parsing") << "Evaluating AST Node: [" << servers[i].GetValue() << "]";

		if (servers[i].GetValue() != "server")
			continue;

		DEBUG("Parsing") << "--> Populating a new Config::Server object.";
		Config::Server srv;

		srv.srvNode = &(servers[i]);
		fillServer(servers[i], srv);
		if (srv.listen.empty())
		{
			srv.listen.push_back("1025");
			srv.hosts.push_back(make_pair("", "1025"));
			DEBUG("Parsing") << "    -> No 'listen' directive found. Defaulting to [*:1025].";
		}
		if (srv.root.empty())
		{
			srv.root = "EngineX/www/";
			createDir();
			DEBUG("Parsing") << "    -> No 'root' directive found. Defaulting to current directory.";

			if (srv.index.empty())
			{
				srv.index.push_back("index.htm");
				DEBUG("Parsing") << "    -> No 'index' directive found. Defaulting to [index.html].";
				if (access("EngineX/www/index.htm", F_OK) == -1)
				{
					writeIndexFile();
				}
			}
		}
		conf.Servers.push_back(srv);

		DEBUG("Parsing") << "<-- Config::Server object populated successfully.";
	}
	DEBUG("Parsing") << "Resolving 'listen to all hosts' configurations.";
	conf.listenToAllHosts();
	INFO() << "Config structure successfully populated.";
}

void Config::parseListen(const string &str, string &port, string &host)
{
	DDEBUG("Parsing") << "Parsing 'listen' directive value: [" << str << "]";

	if (str[0] == '[')
	{
		std::string::size_type endBracket = str.find(']');

		if (endBracket != std::string::npos)
		{
			host = str.substr(1, endBracket - 1);
			if (endBracket + 1 < str.length() && str[endBracket + 1] == ':')
				port = str.substr(endBracket + 2);
			else
				port = "80";

			DDEBUG("Parsing") << "  -> Detected IPv6 format => Host: [" << host << "], Port: [" << port << "]";
			return;
		}
		else
			Error::ThrowError("Invalid host format: " + str);
	}

	const char *colon = strrchr(str.c_str(), ':');

	if (colon != NULL)
	{
		host = str.substr(0, colon - str.c_str());
		port = colon + 1;
		DDEBUG("Parsing") << "  -> Detected 'host:port' format => Host: [" << host << "], Port: [" << port << "]";
		return;
	}

	bool isNumeric = true;
	for (size_t i = 0; i < str.size(); i++)
	{
		if (!isdigit(str[i]))
		{
			isNumeric = false;
			break;
		}
	}

	if (isNumeric)
	{
		port = str;
		DDEBUG("Parsing") << "  -> Detected pure numeric format => Port: [" << port << "] (Host defaults to wildcard)";
	}
	else
	{
		host = str;
		port = "80";
		DDEBUG("Parsing") << "  -> Detected hostname format => Host: [" << host << "], Port: [" << port << "] (default)";
	}
}

void Config::fillServer(AST<string> &serverNode, Config::Server &srv)
{
	vector<AST<string> > &children = serverNode.GetChildren();

	for (int i = 0; i < (int)children.size(); i++)
	{
		AST<string> &node = children[i];
		const string val = node.GetValue();
		vector<string> &args = node.GetArguments();

		if (val == "listen")
		{
			string listen = args[0];
			if (args.size() > 1)
				listen += ":" + args[1];
			if (find(srv.listen.begin(), srv.listen.end(), listen) != srv.listen.end())
				Error::ThrowError("Duplicate host:port in the same server");

			srv.listen.push_back(listen);

			pair<string, string> p;
			parseListen(listen, p.second, p.first);
			srv.hosts.push_back(p);
			DDEBUG("Parsing") << "    -> Bound listen address: [" << listen << "]";
		}
		else if (val == "server_name")
		{
			srv.serverName = args[0];
			DDEBUG("Parsing") << "    -> Set server_name: [" << srv.serverName << "]";
		}
		else if (val == "root")
		{
			srv.root = args[0];
			Utility::getAbsolute(srv.root);
			Utility::normalizePath(srv.root);
			if (srv.root[srv.root.length() - 1] != '/')
				srv.root += '/';
			DDEBUG("Parsing") << "    -> Set root: [" << srv.root << "]";
		}
		else if (val == "index")
		{
			for (size_t idx = 0; idx < args.size(); ++idx)
			{
				Utility::normalizePath(args[idx]);
			}
			srv.index.insert(srv.index.end(), args.begin(), args.end());
			DDEBUG("Parsing") << "    -> Set server_name: [" << srv.serverName << "]";
		}
		else if (val == "autoindex")
		{

			srv.autoindex = (args[0] == "on");
			DDEBUG("Parsing") << "    -> Set autoindex: [" << (srv.autoindex ? "ON" : "OFF") << "]";
		}
		else if (val == "client_max_body_size")
		{
			srv.isMaxBodySize = true;
			srv.clientMaxBodySize = parseByteSize(args[0]);
			DDEBUG("Parsing") << "    -> Set client_max_body_size: [" << srv.clientMaxBodySize << " bytes]";
		}
		else if (val == "allow_methods")
		{
			srv.allowMethodExists = true;
			srv.allowMethods.insert(srv.allowMethods.end(),
									args.begin(), args.end());
			DDEBUG("Parsing") << "    -> Set allow_methods with " << args.size() << " method(s).";
		}
		else if (val == "error_page")
		{
			Utility::normalizePath(args[1]);
			srv.errorPages[args[0]] = args[1];
			DDEBUG("Parsing") << "    -> Mapped error_page: [" << args[0] << "] to [" << args[1] << "]";
		}
		else if (val == "keep_alive_timeout")
		{
			srv.keepAliveTimeout = (int)parseByteSize(args[0]) * USEC;
			DDEBUG("Parsing") << "    -> Set keep_alive_timeout: [" << srv.keepAliveTimeout << "]";
		}
		else if (val == "client_read_timeout")
		{
			srv.clientReadTimeout = (int)parseByteSize(args[0]) * USEC;
			DDEBUG("Parsing") << "    -> Set client_read_timeout: [" << srv.clientReadTimeout << "]";
		}
		else if (val == "cgi_timeout")
		{
			srv.cgiTimeout = (int)parseByteSize(args[0]) * USEC;
			DDEBUG("Parsing") << "    -> Set cgi_timeout: [" << srv.cgiTimeout << "]";
		}
		else if (val == "location")
		{
			Config::Server::Location loc;
			loc.path = node.GetArguments()[0];
			Utility::normalizePath(loc.path);
			if (loc.path[loc.path.length() - 1] != '/')
				loc.path += '/';
			loc.locNode = &node;
			DEBUG("Parsing") << "  --> Populating Location configuration for path: [" << loc.path << "]";

			fillLocation(node, loc);
			srv.Locations.push_back(loc);

			DEBUG("Parsing") << "  <-- Location [" << loc.path << "] populated successfully.";
		}
		else
			Error::ThrowError("Unknown directive in server block: " + val);
	}
}

void Config::fillLocation(AST<string> &locationNode, Config::Server::Location &loc)
{
	Utility::parseBySep(loc.parsedPath, loc.path, "/");
	DDEBUG("Parsing") << "    Parsed location path segments: " << loc.parsedPath.size() << " part(s).";

	vector<AST<string> > &children = locationNode.GetChildren();

	for (int i = 0; i < (int)children.size(); i++)
	{
		AST<string> &node = children[i];
		const string val = node.GetValue();
		vector<string> &args = node.GetArguments();

		DDEBUG("Parsing") << "    [Location] Processing directive [" << i << "]: '" << val << "'";

		if (val == "root")
		{
			loc.root = args[0];
			Utility::getAbsolute(loc.root);
			Utility::normalizePath(loc.root);
			if (loc.root[loc.root.length() - 1] != '/')
				loc.root += '/';
			DDEBUG("Parsing") << "      -> Set root: [" << loc.root << "]";
		}
		else if (val == "index")
		{
			for (size_t idx = 0; idx < args.size(); ++idx)
			{
				Utility::normalizePath(args[idx]);
			}
			loc.index.insert(loc.index.end(), args.begin(), args.end());
			DDEBUG("Parsing") << "      -> Set index with " << args.size() << " file(s).";
		}
		else if (val == "autoindex")
		{
			loc.autoindex = (args[0] == "on");
			DDEBUG("Parsing") << "      -> Set autoindex: [" << (loc.autoindex ? "ON" : "OFF") << "]";
		}
		else if (val == "allow_methods")
		{
			loc.allowMethodExists = true;
			loc.allowMethods.insert(loc.allowMethods.end(),
									args.begin(), args.end());
			DDEBUG("Parsing") << "      -> Set allow_methods with " << args.size() << " method(s).";
		}
		else if (val == "cgi_pass")
		{
			loc.cgiPassExt = args[0];
			loc.cgiPassPath = args[1];
			DDEBUG("Parsing") << "      -> Set cgi_pass: extension=[" << loc.cgiPassExt << "], path=[" << loc.cgiPassPath << "]";
		}
		else if (val == "return")
		{
			loc.returnCode = args[0];
			if (args.size() > 1)
				loc.returnArg = args[1];
			DDEBUG("Parsing") << "      -> Set return: code=[" << loc.returnCode << "]"
							  << (loc.returnArg.empty() ? "" : string(", url=[") + loc.returnArg + "]");
		}
		else if (val == "client_max_body_size")
		{
			loc.isMaxBodySize = true;
			loc.clientMaxBodySize = parseByteSize(args[0]);
			DDEBUG("Parsing") << "      -> Set client_max_body_size: [" << loc.clientMaxBodySize << " bytes]";
		}
		else if (val == "client_body_in_file_only")
		{
			loc.clientBodyInFileOnly = (args[0] == "on");
			DDEBUG("Parsing") << "      -> Set client_body_in_file_only: [" << (loc.clientBodyInFileOnly ? "ON" : "OFF") << "]";
		}
		else if (val == "delete_files")
		{
			loc.deleteFiles = (args[0] == "on");
			DDEBUG("Parsing") << "      -> Set delete_files: [" << (loc.deleteFiles ? "ON" : "OFF") << "]";
		}
		else if (val == "chunked_send")
		{
			loc.chunkedSend = (args[0] == "on");
			DDEBUG("Parsing") << "      -> Set chunked_send: [" << (loc.chunkedSend ? "ON" : "OFF") << "]";
		}
		else if (val == "method_redirect")
		{
			string method = args[0];
			string redirectUrl = args[1];
			Utility::normalizePath(redirectUrl);
			if (redirectUrl[redirectUrl.length() - 1] != '/')
				redirectUrl += '/';
			loc.methodRedirects[method] = redirectUrl;
			DDEBUG("Parsing") << "      -> Set method_redirect: method=[" << method << "], url=[" << redirectUrl << "]";
		}
		else
			Error::ThrowError("Unknown directive in location block: " + val);
	}
	DDEBUG("Parsing") << "    fillLocation complete for path: [" << loc.path << "]";
}

size_t Config::parseByteSize(const string &raw)
{
	char *endptr = NULL;
	size_t value = (size_t)strtoll(raw.c_str(), &endptr, 10);

	if (endptr && *endptr != '\0')
	{
		char unit = *endptr;
		if (unit == 'k' || unit == 'K')
			value *= 1024;
		else if (unit == 'm' || unit == 'M')
			value *= 1024 * 1024;
		else if (unit == 'g' || unit == 'G')
			value *= 1024 * 1024 * 1024;
	}

	return value;
}
