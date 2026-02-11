#pragma once
#include "../Headers.hpp"

class AFd 
{
protected:
	int fd;
	string type;
public:
	long markedTime;
	AFd(int fd, string type);
	virtual void Handle() = 0;
	operator int() const ;
	IContext *context;
	string GetType();
	int GetFd();
	bool MarkedToFree;
	virtual ~AFd();
};