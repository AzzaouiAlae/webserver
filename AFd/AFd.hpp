#pragma once
#include "../Headers.hpp"

class AFd 
{
	int fd;
public:
	AFd(int fd);
	// void 
	operator int() const ;
};