#pragma once

#include "../Headers.hpp"

class Logging {
	ofstream log;
	
public:
	static Logging *current;
	Logging(string filename);
	void PrintLogToScreen(const string &log);
};
