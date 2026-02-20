#include "../Headers.hpp"
#include "../Singleton/Singleton.hpp"

// ═══════════════════════════════════════════════════════════════
//  Constructor
// ═══════════════════════════════════════════════════════════════

Parsing::Parsing(const vector<string> &tokens)
	: _tokens(tokens), _idx(0)
{
}

// ═══════════════════════════════════════════════════════════════
//  PUBLIC – BuildAST
//  Entry point: walk every top-level token and build the tree.
// ═══════════════════════════════════════════════════════════════

void Parsing::BuildAST()
{
	while (!atEnd())
	{
		const string &tok = current();

		if (tok == "server")
			parseServer();
		else if (tok == "types")
			parseTypes();
		else
			Error::ThrowError("Unexpected token at top level: " + tok);
	}
}

// ═══════════════════════════════════════════════════════════════
//  PRIVATE – parseServer
//  Handles:  server { <directives…> }
// ═══════════════════════════════════════════════════════════════

void Parsing::parseServer()
{
	consume(); // eat "server"

	if (current() != "{")
		Error::ThrowError("Expected '{' after server");
	consume(); // eat "{"

	// Add a server node to the root
	AST<string> &root = Singleton::GetASTroot();
	root.AddChild("server");
	AST<string> &serverNode =
		root.GetChildren().back();

	// Parse everything until the matching "}"
	while (!atEnd() && current() != "}")
	{
		if (current() == "location")
			parseLocation(serverNode);
		else
			parseDirective(serverNode);
	}

	if (atEnd())
		Error::ThrowError("Unexpected end of file: missing '}' for server");
	consume(); // eat "}"
}

// ═══════════════════════════════════════════════════════════════
//  PRIVATE – parseLocation
//  Handles:  location /path { <directives…> }
// ═══════════════════════════════════════════════════════════════

void Parsing::parseLocation(AST<string> &server)
{
	consume(); // eat "location"

	// The next token is the path  (e.g. "/", "/api/")
	if (isSeparator())
		Error::ThrowError("Expected a path after 'location'");

	const string path = consume(); // save & eat path token

	if (current() != "{")
		Error::ThrowError("Expected '{' after location path");
	consume(); // eat "{"

	// Add a location child to the server node
	server.AddChild("location");
	AST<string> &locationNode = server.GetChildren().back();
	locationNode.AddArgument(path); // store the path as an argument

	// Parse directives inside the location block
	while (!atEnd() && current() != "}")
		parseDirective(locationNode);

	if (atEnd())
		Error::ThrowError("Unexpected end of file: missing '}' for location");
	consume(); // eat "}"
}

// ═══════════════════════════════════════════════════════════════
//  PRIVATE – parseTypes
//  Handles:  types { mime/type ext… ; … }
//  Populates Singleton::GetMime() directly (same as before).
// ═══════════════════════════════════════════════════════════════

void Parsing::parseTypes()
{
	consume(); // eat "types"

	if (current() != "{")
		Error::ThrowError("Expected '{' after types");
	consume(); // eat "{"

	map<string, string> &mime = Singleton::GetMime();

	while (!atEnd() && current() != "}")
	{
		// first token of each line is the mime type  (e.g. "text/html")
		if (current().find('/') == string::npos)
			Error::ThrowError("Invalid MIME type: " + current());

		const string mimeType = consume();

		// collect all extensions until ";"
		while (!atEnd() && current() != ";")
		{
			if (current() == "}" || current() == "{")
				Error::ThrowError("Invalid Syntax inside types block");
			mime[consume()] = mimeType;
		}

		if (current() != ";")
			Error::ThrowError("Missing ';' in types block");
		consume(); // eat ";"
	}

	if (atEnd())
		Error::ThrowError("Unexpected end of file: missing '}' for types");
	consume(); // eat "}"
}

// ═══════════════════════════════════════════════════════════════
//  PRIVATE – parseDirective
//  Handles:  keyword  value1 value2 … ;
//  Creates a child node on `parent` whose value is the keyword,
//  and stores each value as an argument of that node.
// ═══════════════════════════════════════════════════════════════

void Parsing::parseDirective(AST<string> &parent)
{
	if (isSeparator())
		Error::ThrowError("Unexpected separator: " + current());

	const string keyword = consume(); // e.g. "listen", "root", …

	parent.AddChild(keyword);
	AST<string> &node = parent.GetChildren().back();

	// Collect all tokens until ";"
	while (!atEnd() && current() != ";")
	{
		if (current() == "{" || current() == "}")
			Error::ThrowError("Missing ';' after directive: " + keyword);
		node.AddArgument(consume());
	}

	if (current() != ";")
		Error::ThrowError("Missing ';' after directive: " + keyword);
	consume(); // eat ";"
}

// ═══════════════════════════════════════════════════════════════
//  PUBLIC – FillConf
//  Walks the validated AST and fills Singleton::GetConf().
// ═══════════════════════════════════════════════════════════════

void Parsing::FillConf()
{
	Config &conf = Singleton::GetConf();

	vector<AST<string> > &servers =
		Singleton::GetASTroot().GetChildren();

	for (int i = 0; i < (int)servers.size(); i++)
	{
		// Skip non-server top-level nodes (e.g. types)
		if (servers[i].GetValue() != "server")
			continue;

		Config::Server srv;
		fillServer(servers[i], srv);
		conf.Servers.push_back(srv);
	}

	conf.listenToAllHosts();
}

// ═══════════════════════════════════════════════════════════════
//  PRIVATE – fillServer
// ═══════════════════════════════════════════════════════════════

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
		}
		else if (val == "server_name")
			srv.serverName = args[0];
		else if (val == "root")
			srv.root = args[0];
		else if (val == "index")
			srv.index.insert(srv.index.end(), args.begin(), args.end());
		else if (val == "autoindex")
			srv.autoindex = (args[0] == "on");
		else if (val == "client_max_body_size")
		{
			srv.isMaxBodySize = true;
			srv.clientMaxBodySize = parseByteSize(args[0]);
		}
		else if (val == "allow_methods")
		{
			srv.allowMethodExists = true;
			srv.allowMethods.insert(srv.allowMethods.end(),
									args.begin(), args.end());
		}
		else if (val == "error_page")
			srv.errorPages[args[0]] = args[1];
		else if (val == "return")
		{
			// server-level return: stored as a special location-less entry
			// (handled the same way Validation already accepts it)
		}
		else if (val == "location")
		{
			Config::Server::Location loc;
			loc.path = node.GetArguments()[0]; // path was stored as argument
			fillLocation(node, loc);
			srv.Locations.push_back(loc);
		}
		else
			Error::ThrowError("Unknown directive in server block: " + val);
	}
}

// ═══════════════════════════════════════════════════════════════
//  PRIVATE – fillLocation
// ═══════════════════════════════════════════════════════════════

void Parsing::fillLocation(AST<string> &locationNode,
						   Config::Server::Location &loc)
{
	Utility::parseBySep(loc.parsedPath, loc.path, "/");

	vector<AST<string> > &children = locationNode.GetChildren();

	for (int i = 0; i < (int)children.size(); i++)
	{
		AST<string> &node = children[i];
		const string val = node.GetValue();
		vector<string> &args = node.GetArguments();

		if (val == "root")
			loc.root = args[0];
		else if (val == "index")
			loc.index.insert(loc.index.end(), args.begin(), args.end());
		else if (val == "autoindex")
			loc.autoindex = (args[0] == "on");
		else if (val == "allow_methods")
		{
			loc.allowMethodExists = true;
			loc.allowMethods.insert(loc.allowMethods.end(),
									args.begin(), args.end());
		}
		else if (val == "cgi_pass")
		{
			loc.cgiPassExt = args[0];
			loc.cgiPassPath = args[1];
		}
		else if (val == "return")
		{
			loc.returnCode = args[0];
			if (args.size() > 1)
				loc.returnArg = args[1];
		}
		else if (val == "client_max_body_size")
		{
			loc.isMaxBodySize = true;
			loc.clientMaxBodySize = parseByteSize(args[0]);
		}
		else if (val == "client_body_in_file_only")
			loc.clientBodyInFileOnly = (args[0] == "on");
		else if (val == "delete_files")
			loc.deleteFiles = (args[0] == "on");
		else if (val == "error_page")
			; // location-level error pages could be supported here
		else
			Error::ThrowError("Unknown directive in location block: " + val);
	}
}

// ═══════════════════════════════════════════════════════════════
//  STATIC – parseListen
//  Splits "host:port", "host", or "port" into components.
// ═══════════════════════════════════════════════════════════════

void Parsing::parseListen(const string &str,
						  string &port,
						  string &host)
{
	const char *colon = strrchr(str.c_str(), ':');

	if (colon != NULL)
	{
		host = str.substr(0, colon - str.c_str());
		port = colon + 1;
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
		port = str;
	else
	{
		host = str;
		port = "80";
	}
}

// ═══════════════════════════════════════════════════════════════
//  STATIC – parseByteSize
//  Converts "512k", "10M", "1G", "1024" → bytes.
// ═══════════════════════════════════════════════════════════════

size_t Parsing::parseByteSize(const string &raw)
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

// ═══════════════════════════════════════════════════════════════
//  Token stream helpers
// ═══════════════════════════════════════════════════════════════

const string &Parsing::current() const
{
	return _tokens[_idx];
}

const string &Parsing::consume()
{
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
