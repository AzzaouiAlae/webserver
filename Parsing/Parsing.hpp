#pragma once
#include "../Headers.hpp"

class Parsing
{
	public:
		Parsing(const vector<string> &tokens);

		void BuildAST();


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
		
};
