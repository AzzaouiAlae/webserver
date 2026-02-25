#pragma once
#include "../Headers.hpp"

class Parsing
{
	public:
		Parsing(const vector<string> &tokens);

		void BuildAST();
		void FillConf();

		static void parseListen(const string &str, string &port, string &host);

	private:
		const vector<string> &_tokens;
		int _idx;

		void parseServer();
		void parseLocation(AST<string> &server);
		void parseTypes();
		void parseDirective(AST<string> &parent);

		const string &current() const;
		const string &consume();
		bool isSeparator() const;
		bool atEnd() const;

		void fillServer(AST<string> &serverNode, Config::Server &srv);
		void fillLocation(AST<string> &locationNode, Config::Server::Location &loc);
		
};
