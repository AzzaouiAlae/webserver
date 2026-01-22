#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <string>
#include "../ErrorHandling/Error.hpp"


class Tokenizing
{
	std::string _filepath;
	std::vector<std::string> _tokens;
	std::ifstream _file;
	void openConfFile();
	void trim(std::string& str);
	char shearch_delimiter(std::string& str, std::string delimiters);
	public:
	Tokenizing(std::string filepath);
	const std::vector<std::string>& get_tokens() const;
	void split_tokens();
};