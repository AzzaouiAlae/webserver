#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include "../ErrorHandling/Error.hpp"
#include <stdlib.h>
#include <string>


class Tokenizing
{
	std::string _filepath;
	std::vector<std::string> _tokens;
	std::ifstream _file;
	void openConfFile();
public:
	Tokenizing(std::string filepath);
	const std::vector<std::string>& get_tokens() const;
	void split_tokens();
};