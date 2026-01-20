#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include "../ErrorHandling/Error.hpp"
#include <stdlib.h>
#include <string>

enum TOKENS
{
	SERVER,
	LISTEN,
	SERVER_NAME,
	ROOT,
	INDEX,
	ERROR_PAGE,
	CLIENT_MAX_BODY_SIZE,
	LOCATION,

	
	
	METHODS,
	AUTOINDEX,
	RETURN,
	UPLOAD_PATH,
	CGI_PASS,
	CGI_EXTENSION,

	GET,
	POST,
	DELETE,

	ON,
	OFF,
	ERROR_STATE,

	BLOCK_START,
	END_BLOCK,
	SEMI_COL
};

class Tokenizing
{
	std::string _filepath;
	std::vector<std::string> _tokens;
	std::ofstream _file;
	void openConfFile();
public:
	Tokenizing(std::string filepath);
	const std::vector<std::string>& get_tokens() const;
	void split_tokens();
};