#include "Config.hpp"
#include "../Socket/Socket.hpp"

Config::Server::Server()
{
	autoindex = true;
	clientMaxBodySize = 0;
	allowMethodExists = false;
	isMaxBodySize = false;
}

Config::Server::Location::Location()
{
	allowMethodExists = false;
	clientMaxBodySize = 0;
	isMaxBodySize = false;
	deleteFiles = false;
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

int Config::GetMaxBodySize(vector<Config::Server> &srvs)
{
	size_t size = 0;
	for (int i = 0; i < (int)srvs.size(); i++)
	{
		if (size < srvs[i].clientMaxBodySize) {
			size = srvs[i].clientMaxBodySize;
		}
		vector < Config::Server::Location> &loc = srvs[i].Locations;
		for(int j = 0; j < (int)loc.size(); j++)
		{
			if (size < loc[j].clientMaxBodySize) {
				size = loc[j].clientMaxBodySize;
			}
		}
	}
	return size;
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

// Checks if locPath is a prefix of reqPath
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

// Checks if the request path ends with the CGI extension
bool Config::pathMatchesCgiExt(const string &path, const string &cgiPassExt)
{
    if (path.length() < cgiPassExt.length())
        return false;
    return path.compare(path.length() - cgiPassExt.length(),
                        cgiPassExt.length(), cgiPassExt) == 0;
}

// Returns the index of the best (longest prefix) CGI location match, or -1
int Config::findBestCgiMatch(Config::Server &srv, const vector<string> &reqPath, const string &path)
{
    int    bestIndex = -1;
    size_t maxLength = 0;

    for (size_t i = 0; i < srv.Locations.size(); ++i)
    {
        const Config::Server::Location &loc = srv.Locations[i];
        if (loc.cgiPassExt.empty())
            continue;
        if (!isPrefixMatch(loc.parsedPath, reqPath))
            continue;
        if (!pathMatchesCgiExt(path, loc.cgiPassExt))
            continue;
        if (loc.parsedPath.size() > maxLength || bestIndex == -1)
        {
            maxLength = loc.parsedPath.size();
            bestIndex = (int)i;
        }
    }
    return bestIndex;
}

// Returns the index of the best (longest prefix) static location match, or -1
int Config::findBestStaticMatch(Config::Server &srv, const vector<string> &reqPath)
{
    int    bestIndex = -1;
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

// Main orchestrator — now just coordinates the helpers
int Config::GetLocationIndex(Config::Server &srv, const string &path)
{
    vector<string> reqPath;
    Utility::parseBySep(reqPath, path, "/");

    int bestCgiIndex = findBestCgiMatch(srv, reqPath, path);
    if (bestCgiIndex != -1)
	{
        DDEBUG("Config") << "Location match for '" << path << "': CGI location index " << bestCgiIndex;
        return bestCgiIndex;
	}

    int bestStaticIndex = findBestStaticMatch(srv, reqPath);
    DDEBUG("Config") << "Location match for '" << path << "': static location index " << bestStaticIndex;
    return bestStaticIndex;
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

		// Skip non-server top-level nodes (e.g. types)
		if (servers[i].GetValue() != "server")
			continue;

		DEBUG("Parsing") << "--> Populating a new Config::Server object.";
		Config::Server srv;

		srv.srvNode = &(servers[i]);
		fillServer(servers[i], srv);
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

	const char *colon = strrchr(str.c_str(), ':');

	if (colon != NULL)
	{
		host = str.substr(0, colon - str.c_str());
		port = colon + 1;
		DDEBUG("Parsing") << "  -> Detected 'host:port' format => Host: [" << host << "], Port: [" << port << "]";
		return;
	}

	// No colon — decide if it is a pure numeric port or a hostname
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
			if (find(srv.listen.begin(), srv.listen.end(), args[0]) != srv.listen.end())
				Error::ThrowError("Duplicate host:port in the same server");

			srv.listen.push_back(args[0]);

			pair<string, string> p;
			parseListen(args[0], p.second, p.first);
			srv.hosts.push_back(p);
			DDEBUG("Parsing") << "    -> Bound listen address: [" << args[0] << "]";
		}
		else if (val == "server_name") {
			srv.serverName = args[0];
			DDEBUG("Parsing") << "    -> Set server_name: [" << srv.serverName << "]";
		}
		else if (val == "root") {
			srv.root = args[0];
			DDEBUG("Parsing") << "    -> Set root: [" << srv.root << "]";
		}
		else if (val == "index") {
			srv.index.insert(srv.index.end(), args.begin(), args.end());
			DDEBUG("Parsing") << "    -> Set server_name: [" << srv.serverName << "]";
		}
		else if (val == "autoindex") {
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
		else if (val == "error_page") {
			srv.errorPages[args[0]] = args[1];
			DDEBUG("Parsing") << "    -> Mapped error_page: [" << args[0] << "] to [" << args[1] << "]";
		}
		else if (val == "location")
		{
			Config::Server::Location loc;
			loc.path = node.GetArguments()[0]; // path was stored as argument
			
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

void Config::fillLocation(AST<string> &locationNode,
						   Config::Server::Location &loc)
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
			DDEBUG("Parsing") << "      -> Set root: [" << loc.root << "]";
		}
		else if (val == "index")
		{
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
		else if (val == "error_page")
		{
			DDEBUG("Parsing") << "      -> Skipping location-level error_page (not yet supported).";
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
