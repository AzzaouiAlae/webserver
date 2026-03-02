#pragma once
#include "Headers.hpp"

class APipe: public AFd
{
public:
	APipe(int fd, string type);
	virtual ~APipe() {}
};