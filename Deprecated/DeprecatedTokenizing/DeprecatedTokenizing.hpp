#pragma once
#include "../../Headers.hpp"

class DeprecatedTokenizing
{
	string _filepath;
	vector<string> _tokens;
	ifstream _file;
	void openConfFile();
	char shearch_delimiter(string& str, string delimiters);
public:
	DeprecatedTokenizing(string filepath);
	const vector<string>& get_tokens() const;
	void split_tokens();
};
