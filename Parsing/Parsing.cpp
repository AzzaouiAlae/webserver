#include "Parsing.hpp"

Parsing::Parsing(const vector<string> &tokens)
	: _tokens(tokens), _idx(0)
{
	DEBUG("Parsing") << "Parsing initialized with " << _tokens.size() << " tokens.";
}

void Parsing::BuildAST()
{
	DEBUG("Parsing") << "Starting AST construction from tokens.";

	while (!atEnd())
	{
		const string &tok = current();

		DDEBUG("Parsing") << "Evaluating top-level token: [" << tok << "]";

		if (tok == "server")
			parseServer();
		else if (tok == "types")
			parseTypes();
		else
			Error::ThrowError("Unexpected token at top level: " + tok);
	}
	DEBUG("Parsing") << "AST construction complete.";
}

void Parsing::parseServer()
{
	DEBUG("Parsing") << "--> Entering 'server' block";
	consume();

	if (current() != "{")
		Error::ThrowError("Expected '{' after server");
	consume();

	DDEBUG("Parsing") << "  -> Creating 'server' node in AST";
	AST<string> &root = Singleton::GetASTroot();
	root.AddChild("server");
	AST<string> &serverNode = root.GetChildren().back();

	while (!atEnd() && current() != "}")
	{
		DDEBUG("Parsing") << "  [Server Block] Evaluating token: [" << current() << "]";

		if (current() == "location")
			parseLocation(serverNode);
		else
			parseDirective(serverNode);
	}
	if (atEnd())
		Error::ThrowError("Unexpected end of file: missing '}' for server");
	consume();
	DEBUG("Parsing") << "<-- Exiting 'server' block";
}

void Parsing::parseLocation(AST<string> &server)
{
	DEBUG("Parsing") << "  --> Entering 'location' block";
	consume();

	if (isSeparator())
		Error::ThrowError("Expected a path after 'location'");

	const string path = consume();
	DDEBUG("Parsing") << "    -> Location path detected: [" << path << "]";

	if (current() != "{")
		Error::ThrowError("Expected '{' after location path");
	consume();

	server.AddChild("location");
	AST<string> &locationNode = server.GetChildren().back();
	locationNode.AddArgument(path);

	while (!atEnd() && current() != "}")
		parseDirective(locationNode);

	if (atEnd())
		Error::ThrowError("Unexpected end of file: missing '}' for location");
	consume();
	DEBUG("Parsing") << "  <-- Exiting 'location' block";
}

void Parsing::parseTypes()
{
	DEBUG("Parsing") << "--> Entering 'types' block";
	consume();

	if (current() != "{")
		Error::ThrowError("Expected '{' after types");
	consume();

	map<string, string> &mime = Singleton::GetMime();

	while (!atEnd() && current() != "}")
	{
		DDEBUG("Parsing") << "  [Types Block] Evaluating token: [" << current() << "]";

		if (current().find('/') == string::npos)
			Error::ThrowError("Invalid MIME type: " + current());

		const string mimeType = consume();
		DDEBUG("Parsing") << "    -> MIME type detected: [" << mimeType << "]";

		while (!atEnd() && current() != ";")
		{
			if (current() == "}" || current() == "{")
				Error::ThrowError("Invalid Syntax inside types block");
			string ext = consume();
			DDEBUG("Parsing") << "      -> Mapped extension: [" << ext << "] to [" << mimeType << "]";
			mime[ext] = mimeType;
		}

		if (current() != ";")
			Error::ThrowError("Missing ';' in types block");
		consume();
	}

	if (atEnd())
		Error::ThrowError("Unexpected end of file: missing '}' for types");
	consume();
	DEBUG("Parsing") << "<-- Exiting 'types' block";
}

void Parsing::parseDirective(AST<string> &parent)
{
	if (isSeparator())
		Error::ThrowError("Unexpected separator: " + current());

	const string keyword = consume();
	DDEBUG("Parsing") << "  -> Parsing directive: [" << keyword << "]";

	parent.AddChild(keyword);
	AST<string> &node = parent.GetChildren().back();

	while (!atEnd() && current() != ";")
	{
		if (current() == "{" || current() == "}")
			Error::ThrowError("Missing ';' after directive: " + keyword);
		string arg = consume();
        DDEBUG("Parsing") << "    -> Argument added to [" << keyword << "]: [" << arg << "]";
        node.AddArgument(arg);
	}

	if (current() != ";")
		Error::ThrowError("Missing ';' after directive: " + keyword);
	if ( node.GetArguments().empty() )
		Error::ThrowError("Directive [" + keyword + "] requires at least one argument");
	consume();
	DDEBUG("Parsing") << "  <- Directive [" << keyword << "] successfully parsed.";
}

void Parsing::FillConf()
{
	DEBUG("Parsing") << "Starting to fill Config structure from AST.";
	Config &conf = Singleton::GetConf();

	vector<AST<string> > &servers = Singleton::GetASTroot().GetChildren();

	for (int i = 0; i < (int)servers.size(); i++)
	{
		DDEBUG("Parsing") << "Evaluating AST Node: [" << servers[i].GetValue() << "]";

		if (servers[i].GetValue() != "server")
			continue;

		DEBUG("Parsing") << "--> Populating a new Config::Server object.";
		Config::Server srv;

		fillServer(servers[i], srv);
		conf.Servers.push_back(srv);

		DEBUG("Parsing") << "<-- Config::Server object populated successfully.";
	}
	DEBUG("Parsing") << "Resolving 'listen to all hosts' configurations.";
	conf.listenToAllHosts();
	INFO() << "Config structure successfully populated.";
}

void Parsing::fillServer(AST<string> &serverNode, Config::Server &srv)
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
			srv.clientMaxBodySize = Utility::parseByteSize(args[0]);
			DDEBUG("Parsing") << "    -> Set client_max_body_size: [" << srv.clientMaxBodySize << " bytes]";
		}
		else if (val == "allow_methods")
		{
			srv.allowMethodExists = true;
			srv.allowMethods.insert(srv.allowMethods.end(), args.begin(), args.end());
			DDEBUG("Parsing") << "    -> Set allow_methods with " << args.size() << " method(s).";
		}
		else if (val == "error_page") {
			srv.errorPages[args[0]] = args[1];
			DDEBUG("Parsing") << "    -> Mapped error_page: [" << args[0] << "] to [" << args[1] << "]";
		}
		else if (val == "location")
		{
			Config::Server::Location loc;
			loc.path = node.GetArguments()[0];
			
			DEBUG("Parsing") << "  --> Populating Location configuration for path: [" << loc.path << "]";
			
			fillLocation(node, loc);
			srv.Locations.push_back(loc);
			
			DEBUG("Parsing") << "  <-- Location [" << loc.path << "] populated successfully.";
		}
		else
			Error::ThrowError("Unknown directive in server block: " + val);
	}
}

void Parsing::fillLocation(AST<string> &locationNode, Config::Server::Location &loc)
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
			loc.allowMethods.insert(loc.allowMethods.end(), args.begin(), args.end());
			DDEBUG("Parsing") << "      -> Set allow_methods with " << args.size() << " method(s).";
		}
		else if (val == "cgi_pass")
		{
			if (args.size() < 2)
				Error::ThrowError("cgi_pass directive requires at least 2 arguments: extension and path");
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
			loc.clientMaxBodySize = Utility::parseByteSize(args[0]);
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

void Parsing::parseListen(const string &str, string &port, string &host)
{
	DDEBUG("Parsing") << "Parsing 'listen' directive value: [" << str << "]";


	std::string::size_type pos = str.rfind(':');

	if (pos != std::string::npos)
	{
	    host = str.substr(0, pos);
	    port = str.substr(pos + 1);

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

//  --- helpers ---
const string &Parsing::current() const
{
    if (atEnd())
    {
        Error::ThrowError("Unexpected end of file");
    }
	return _tokens[_idx];
}

const string &Parsing::consume()
{
	DDEBUG("Parsing") << "  [Consume] -> '" << _tokens[_idx] << "'";
	return _tokens[_idx++];
}

bool Parsing::isSeparator() const
{
	const string &t = _tokens[_idx];
	return (t == ";" || t == "{" || t == "}");
}

bool Parsing::atEnd() const
{
	return _idx >= (int)_tokens.size();
}
