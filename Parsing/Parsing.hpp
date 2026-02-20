#pragma once
#include "../Headers.hpp"

// ─────────────────────────────────────────────────────────────
//  Parsing
//
//  Responsibility 1 – BuildAST()
//    Walks the flat token list produced by Tokenizing and builds
//    a hierarchical AST stored in Singleton::GetASTroot().
//    It understands block structure (server{}, location{}) but
//    does NOT validate values — that is Validation's job.
//
//  Responsibility 2 – FillConf()
//    Walks the completed (and already validated) AST and fills
//    the runtime Config object used by the rest of the server.
// ─────────────────────────────────────────────────────────────
class Parsing
{
public:
	Parsing(const vector<string> &tokens);

	void BuildAST();
	void FillConf();

	static void parseListen(const string &str,
							string &port,
							string &host);

private:
	// ── token stream state ──────────────────────────────────
	const vector<string> &_tokens;
	int _idx;

	// ── AST building helpers ────────────────────────────────
	void parseServer();						  // server { … }
	void parseLocation(AST<string> &server);  // location /path { … }
	void parseTypes();						  // types { … }
	void parseDirective(AST<string> &parent); // key val… ;

	// ── token helpers ────────────────────────────────────────
	const string &current() const;
	const string &consume();  // return current, advance _idx
	bool isSeparator() const; // ; { }
	bool atEnd() const;

	// ── FillConf helpers ─────────────────────────────────────
	void fillServer(AST<string> &serverNode, Config::Server &srv);
	void fillLocation(AST<string> &locationNode,
					  Config::Server::Location &loc);
	static size_t parseByteSize(const string &raw);
};
