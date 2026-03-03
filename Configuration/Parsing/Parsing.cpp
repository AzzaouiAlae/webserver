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
