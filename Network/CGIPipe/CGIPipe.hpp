#pragma once
#include "Headers.hpp"
#include "Cgi.hpp"

class CGIPipe : public APipe {
	Cgi *cgi;
public:
	CGIPipe(int fd, Cgi *cgi);
	void Handle();
};
