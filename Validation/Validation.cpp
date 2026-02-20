#include "Validation.hpp"

// ═══════════════════════════════════════════════════════════════
//  Constructor
// ═══════════════════════════════════════════════════════════════

Validation::Validation(AST<string> &astRoot)
	: _root(astRoot)
{
}

// ═══════════════════════════════════════════════════════════════
//  PUBLIC – Validate
//  Walk every top-level node in the AST root and validate it.
// ═══════════════════════════════════════════════════════════════

void Validation::Validate()
{
	vector<AST<string> > &topLevel = _root.GetChildren();

	if (topLevel.empty())
		Error::ThrowError("Config Error: no server blocks found");

	for (int i = 0; i < (int)topLevel.size(); i++)
	{
		const string &kind = topLevel[i].GetValue();

		if (kind == "server")
			validateServer(topLevel[i]);
		else if (kind == "types")
			; // types block is structural — already parsed, nothing to validate here
		else
			Error::ThrowError("Config Error: unexpected top-level block '" + kind + "'");
	}
}

// ═══════════════════════════════════════════════════════════════
//  PRIVATE – validateServer
//  Checks one complete server { … } node.
// ═══════════════════════════════════════════════════════════════

void Validation::validateServer(AST<string> &serverNode)
{
	ServerSeen seen;

	vector<AST<string> > &children = serverNode.GetChildren();

	for (int i = 0; i < (int)children.size(); i++)
	{
		AST<string> &node = children[i];
		const string &directive = node.GetValue();

		if (directive == "listen")
			validateListen(node, seen);
		else if (directive == "server_name")
			validateServerName(node, seen);
		else if (directive == "root")
			validateRoot(node, seen.root);
		else if (directive == "index")
			validateIndex(node, seen.index);
		else if (directive == "autoindex")
			validateAutoindex(node, seen.autoindex);
		else if (directive == "return")
			validateReturn(node, seen.returnDir);
		else if (directive == "client_max_body_size")
			validateClientMaxBody(node, seen.clientMaxBodySize);
		else if (directive == "allow_methods")
			validateAllowMethods(node, seen.allowMethods);
		else if (directive == "error_page")
			validateErrorPage(node, seen.errorPage);
		else if (directive == "location")
		{
			// location is a nested block — validate it separately
			// its path is stored as its first argument (set by Parsing)
			if (node.GetArguments().empty())
				Error::ThrowError("Config Error: location block has no path");
			const string &path = node.GetArguments()[0];
			if (path.empty() || path[0] != '/')
				Error::ThrowError("Config Error: location path must start with '/' — got '" + path + "'");

			LocationSeen locSeen;
			validateLocation(node, locSeen);
		}
		else
			Error::ThrowError("Config Error: unknown directive '" + directive + "' in server block");
	}

	// ── Required field checks ─────────────────────────────────
	if (!seen.listen)
		Error::ThrowError("Config Error: server block is missing required 'listen' directive");
	if (!seen.serverName)
		Error::ThrowError("Config Error: server block is missing required 'server_name' directive");
}

// ═══════════════════════════════════════════════════════════════
//  PRIVATE – validateLocation
//  Checks one location { … } node.
// ═══════════════════════════════════════════════════════════════

void Validation::validateLocation(AST<string> &locationNode,
								  LocationSeen &seen)
{
	vector<AST<string> > &children = locationNode.GetChildren();

	for (int i = 0; i < (int)children.size(); i++)
	{
		AST<string> &node = children[i];
		const string &directive = node.GetValue();

		if (directive == "root")
			validateRoot(node, seen.root);
		else if (directive == "index")
			validateIndex(node, seen.index);
		else if (directive == "autoindex")
			validateAutoindex(node, seen.autoindex);
		else if (directive == "return")
			validateReturn(node, seen.returnDir);
		else if (directive == "client_max_body_size")
			validateClientMaxBody(node, seen.clientMaxBodySize);
		else if (directive == "allow_methods")
			validateAllowMethods(node, seen.allowMethods);
		else if (directive == "cgi_pass")
			validateCgiPass(node, seen);
		else if (directive == "client_body_in_file_only")
			validateBodyInFile(node, seen);
		else if (directive == "delete_files")
			validateDeleteFiles(node, seen);
		else
			Error::ThrowError("Config Error: unknown directive '" + directive + "' in location block");
	}
}

// ═══════════════════════════════════════════════════════════════
//  SERVER-SCOPE DIRECTIVE VALIDATORS
// ═══════════════════════════════════════════════════════════════

// ── listen ────────────────────────────────────────────────────
// Allowed formats: "8080", "localhost", "0.0.0.0:8080"
// Multiple listen directives are allowed in the same server block.

void Validation::validateListen(AST<string> &node, ServerSeen &seen)
{
	seen.listen = true;
	checkArgCount(node, 1, 1, "listen");

	const string &val = node.GetArguments()[0];
	string port;
	string host; // unused here, just validates the split

	// Find the port part
	const char *colon = strrchr(val.c_str(), ':');
	if (colon != NULL)
		port = string(colon + 1);
	else
	{
		// Check if it's purely numeric (port) or a hostname
		bool numeric = true;
		for (size_t i = 0; i < val.size(); i++)
			if (!isdigit(val[i]))
			{
				numeric = false;
				break;
			}
		port = numeric ? val : "80";
	}

	long portNum = parseNumber(port);
	if (portNum < 1 || portNum > 65535)
		Error::ThrowError("Config Error: listen port out of range (1-65535): " + port);
}

// ── server_name ───────────────────────────────────────────────

void Validation::validateServerName(AST<string> &node, ServerSeen &seen)
{
	if (seen.serverName)
		Error::ThrowError("Config Error: duplicate 'server_name' in server block");
	seen.serverName = true;
	requireArgs(node, "server_name");
}

// ── root ──────────────────────────────────────────────────────
// Shared between server and location scope via bool& seen

void Validation::validateRoot(AST<string> &node, bool &seen)
{
	if (seen)
		Error::ThrowError("Config Error: duplicate 'root' directive");
	seen = true;
	checkArgCount(node, 1, 1, "root");
}

// ── index ─────────────────────────────────────────────────────

void Validation::validateIndex(AST<string> &node, bool &seen)
{
	if (seen)
		Error::ThrowError("Config Error: duplicate 'index' directive");
	seen = true;
	requireArgs(node, "index");

	const vector<string> &args = node.GetArguments();
	for (size_t i = 0; i < args.size(); i++)
	{
		if (args[i].find('/') != string::npos)
			Error::ThrowError("Config Error: index value must not be an absolute path: '" + args[i] + "'");
	}
}

// ── autoindex ─────────────────────────────────────────────────

void Validation::validateAutoindex(AST<string> &node, bool &seen)
{
	if (seen)
		Error::ThrowError("Config Error: duplicate 'autoindex' directive");
	seen = true;
	checkArgCount(node, 1, 1, "autoindex");

	const string &val = node.GetArguments()[0];
	if (val != "on" && val != "off")
		Error::ThrowError("Config Error: autoindex value must be 'on' or 'off', got '" + val + "'");
}

// ── return ────────────────────────────────────────────────────

void Validation::validateReturn(AST<string> &node, bool &seen)
{
	if (seen)
		Error::ThrowError("Config Error: duplicate 'return' directive");
	seen = true;
	checkArgCount(node, 1, 2, "return");

	long code = parseNumber(node.GetArguments()[0]);
	if (code != 200 && code != 301 && code != 302)
		Error::ThrowError("Config Error: return code must be 200, 301, or 302 — got '" +
						  node.GetArguments()[0] + "'");
}

// ── client_max_body_size ──────────────────────────────────────

void Validation::validateClientMaxBody(AST<string> &node, bool &seen)
{
	if (seen)
		Error::ThrowError("Config Error: duplicate 'client_max_body_size' directive");
	seen = true;
	checkArgCount(node, 1, 1, "client_max_body_size");

	const string &val = node.GetArguments()[0];
	if (val.empty())
		Error::ThrowError("Config Error: client_max_body_size has empty value");

	char unit = val[val.size() - 1];
	if (!isByteSizeUnit(unit))
		Error::ThrowError("Config Error: client_max_body_size must end with a unit (b/k/m/g): '" + val + "'");

	// Validate the numeric part
	parseNumber(val.substr(0, val.size() - 1));
}

// ── allow_methods ─────────────────────────────────────────────

void Validation::validateAllowMethods(AST<string> &node, bool &seen)
{
	if (seen)
		Error::ThrowError("Config Error: duplicate 'allow_methods' directive");
	seen = true;
	requireArgs(node, "allow_methods");

	const vector<string> &args = node.GetArguments();
	for (size_t i = 0; i < args.size(); i++)
	{
		if (!isValidMethod(args[i]))
			Error::ThrowError("Config Error: invalid HTTP method '" + args[i] +
							  "' — allowed: GET, POST, DELETE");
	}
}

// ── error_page ────────────────────────────────────────────────

void Validation::validateErrorPage(AST<string> &node, bool &seen)
{
	seen = true; // multiple error_page directives are allowed — no duplicate check
	checkArgCount(node, 2, 2, "error_page");

	long code = parseNumber(node.GetArguments()[0]);
	bool validCode = (code >= 400 && code <= 404) || (code >= 500 && code <= 504);
	if (!validCode)
		Error::ThrowError("Config Error: error_page code must be 4xx or 5xx — got '" +
						  node.GetArguments()[0] + "'");
}

// ═══════════════════════════════════════════════════════════════
//  LOCATION-ONLY DIRECTIVE VALIDATORS
// ═══════════════════════════════════════════════════════════════

// ── cgi_pass ──────────────────────────────────────────────────

void Validation::validateCgiPass(AST<string> &node, LocationSeen &seen)
{
	if (seen.cgiPass)
		Error::ThrowError("Config Error: duplicate 'cgi_pass' in location block");
	if (seen.bodyInFile)
		Error::ThrowError("Config Error: 'cgi_pass' cannot be used with 'client_body_in_file_only'");
	if (seen.deleteFiles)
		Error::ThrowError("Config Error: 'cgi_pass' cannot be used with 'delete_files'");

	seen.cgiPass = true;
	checkArgCount(node, 2, 2, "cgi_pass");

	// First arg is the extension, must contain a dot
	const string &ext = node.GetArguments()[0];
	if (ext.find('.') == string::npos)
		Error::ThrowError("Config Error: cgi_pass extension must contain a dot — got '" + ext + "'");
}

// ── client_body_in_file_only ──────────────────────────────────

void Validation::validateBodyInFile(AST<string> &node, LocationSeen &seen)
{
	if (seen.bodyInFile)
		Error::ThrowError("Config Error: duplicate 'client_body_in_file_only' in location block");
	if (seen.cgiPass)
		Error::ThrowError("Config Error: 'client_body_in_file_only' cannot be used with 'cgi_pass'");

	seen.bodyInFile = true;
	checkArgCount(node, 1, 1, "client_body_in_file_only");

	const string &val = node.GetArguments()[0];
	if (val != "on" && val != "off")
		Error::ThrowError("Config Error: client_body_in_file_only must be 'on' or 'off' — got '" + val + "'");
}

// ── delete_files ──────────────────────────────────────────────

void Validation::validateDeleteFiles(AST<string> &node, LocationSeen &seen)
{
	if (seen.deleteFiles)
		Error::ThrowError("Config Error: duplicate 'delete_files' in location block");
	if (seen.cgiPass)
		Error::ThrowError("Config Error: 'delete_files' cannot be used with 'cgi_pass'");

	seen.deleteFiles = true;
	checkArgCount(node, 1, 1, "delete_files");

	const string &val = node.GetArguments()[0];
	if (val != "on" && val != "off")
		Error::ThrowError("Config Error: delete_files must be 'on' or 'off' — got '" + val + "'");
}

// ═══════════════════════════════════════════════════════════════
//  STATIC VALUE HELPERS
// ═══════════════════════════════════════════════════════════════

long Validation::parseNumber(const string &s)
{
	if (s.empty())
		Error::ThrowError("Config Error: expected a number, got empty string");

	char *endptr = NULL;
	errno = 0;
	long val = strtol(s.c_str(), &endptr, 10);

	if (errno != 0 || endptr == s.c_str() || *endptr != '\0' || s[0] == '+')
		Error::ThrowError("Config Error: invalid number '" + s + "'");

	return val;
}

bool Validation::isByteSizeUnit(char c)
{
	return (c == 'b' || c == 'B' ||
			c == 'k' || c == 'K' ||
			c == 'm' || c == 'M' ||
			c == 'g' || c == 'G');
}

bool Validation::isValidMethod(const string &m)
{
	return (m == "GET" || m == "POST" || m == "DELETE");
}

// C++98-compatible int → string conversion
static string intToStr(int n)
{
	ostringstream oss;
	oss << n;
	return oss.str();
}

// Throws if the node has fewer than min or more than max arguments.
void Validation::checkArgCount(AST<string> &node,
							   size_t min, size_t max,
							   const string &name)
{
	size_t count = node.GetArguments().size();
	if (count < min)
		Error::ThrowError("Config Error: '" + name + "' expects at least " +
						  intToStr((int)min) + " argument(s), got " +
						  intToStr((int)count));
	if (count > max)
		Error::ThrowError("Config Error: '" + name + "' expects at most " +
						  intToStr((int)max) + " argument(s), got " +
						  intToStr((int)count));
}

// Throws if the node has zero arguments.
void Validation::requireArgs(AST<string> &node,
							 const string &name)
{
	if (node.GetArguments().empty())
		Error::ThrowError("Config Error: '" + name + "' requires at least one argument");
}
