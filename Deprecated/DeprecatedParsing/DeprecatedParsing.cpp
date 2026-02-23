#include "Headers.hpp"
#include "Singleton.hpp"

// ═══════════════════════════════════════════════════════════════
//  Constructor
// ═══════════════════════════════════════════════════════════════

DeprecatedParsing::DeprecatedParsing(const vector<string> &tokens)
	: _tokens(tokens), _idx(0)
{
	DEBUG("Parsing") << "Parsing initialized with " << _tokens.size() << " tokens.";
}

// ═══════════════════════════════════════════════════════════════
//  PUBLIC – BuildAST
//  Entry point: walk every top-level token and build the tree.
// ═══════════════════════════════════════════════════════════════

void DeprecatedParsing::BuildAST()
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

// ═══════════════════════════════════════════════════════════════
//  PRIVATE – parseServer
//  Handles:  server { <directives…> }
// ═══════════════════════════════════════════════════════════════

void DeprecatedParsing::parseServer()
{
	DEBUG("Parsing") << "--> Entering 'server' block";
	consume(); // eat "server"

	if (current() != "{")
		Error::ThrowError("Expected '{' after server");
	consume(); // eat "{"

	// Add a server node to the root
	DDEBUG("Parsing") << "  -> Creating 'server' node in AST";
	AST<string> &root = Singleton::GetASTroot();
	root.AddChild("server");
	AST<string> &serverNode =
		root.GetChildren().back();

	// Parse everything until the matching "}"
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
	consume(); // eat "}"
	DEBUG("Parsing") << "<-- Exiting 'server' block";
}

// ═══════════════════════════════════════════════════════════════
//  PRIVATE – parseLocation
//  Handles:  location /path { <directives…> }
// ═══════════════════════════════════════════════════════════════

void DeprecatedParsing::parseLocation(AST<string> &server)
{
	DEBUG("Parsing") << "  --> Entering 'location' block";
	consume(); // eat "location"

	// The next token is the path  (e.g. "/", "/api/")
	if (isSeparator())
		Error::ThrowError("Expected a path after 'location'");

	const string path = consume(); // save & eat path token
	DDEBUG("Parsing") << "    -> Location path detected: [" << path << "]";

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
	DEBUG("Parsing") << "  <-- Exiting 'location' block";
}

// ═══════════════════════════════════════════════════════════════
//  PRIVATE – parseTypes
//  Handles:  types { mime/type ext… ; … }
//  Populates Singleton::GetMime() directly (same as before).
// ═══════════════════════════════════════════════════════════════

void DeprecatedParsing::parseTypes()
{
	DEBUG("Parsing") << "--> Entering 'types' block";
	consume(); // eat "types"

	if (current() != "{")
		Error::ThrowError("Expected '{' after types");
	consume(); // eat "{"

	map<string, string> &mime = Singleton::GetMime();

	while (!atEnd() && current() != "}")
	{
		DDEBUG("Parsing") << "  [Types Block] Evaluating token: [" << current() << "]";

		// first token of each line is the mime type  (e.g. "text/html")
		if (current().find('/') == string::npos)
			Error::ThrowError("Invalid MIME type: " + current());

		const string mimeType = consume();
		DDEBUG("Parsing") << "    -> MIME type detected: [" << mimeType << "]";

		// collect all extensions until ";"
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
		consume(); // eat ";"
	}

	if (atEnd())
		Error::ThrowError("Unexpected end of file: missing '}' for types");
	consume(); // eat "}"
	DEBUG("Parsing") << "<-- Exiting 'types' block";
}

// ═══════════════════════════════════════════════════════════════
//  PRIVATE – parseDirective
//  Handles:  keyword  value1 value2 … ;
//  Creates a child node on `parent` whose value is the keyword,
//  and stores each value as an argument of that node.
// ═══════════════════════════════════════════════════════════════

void DeprecatedParsing::parseDirective(AST<string> &parent)
{
	if (isSeparator())
		Error::ThrowError("Unexpected separator: " + current());

	const string keyword = consume(); // e.g. "listen", "root", …
	DDEBUG("Parsing") << "  -> Parsing directive: [" << keyword << "]";

	parent.AddChild(keyword);
	AST<string> &node = parent.GetChildren().back();

	// Collect all tokens until ";"
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
	consume(); // eat ";"
	DDEBUG("Parsing") << "  <- Directive [" << keyword << "] successfully parsed.";
}

// ═══════════════════════════════════════════════════════════════
//  Token stream helpers
// ═══════════════════════════════════════════════════════════════

const string &DeprecatedParsing::current() const
{
	// Safety check to prevent Segfaults if config file ends unexpectedly
    if (atEnd())
    {
        Error::ThrowError("Unexpected end of file");
    }
	return _tokens[_idx];
}

const string &DeprecatedParsing::consume()
{
	DDEBUG("Parsing") << "  [Consume] -> '" << _tokens[_idx] << "'";
	return _tokens[_idx++];
}

bool DeprecatedParsing::isSeparator() const
{
	const string &t = _tokens[_idx];
	return (t == ";" || t == "{" || t == "}");
}

bool DeprecatedParsing::atEnd() const
{
	return _idx >= (int)_tokens.size();
}
