#pragma once
#include "Headers.hpp"

class APipe: public AFd
{
public:
	static bool GetPipe(int pipe[2]);
	APipe(int fd, string type);
	virtual ~APipe() {}
};