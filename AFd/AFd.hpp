#pragma once
#include "../Headers.hpp"

class AFd 
{
protected:
	int fd;
public:
	AFd(int fd);
	virtual void Handle() = 0;
	operator int() const ;
};