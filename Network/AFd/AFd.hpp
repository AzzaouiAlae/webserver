#pragma once
#include <iostream>
using namespace std;

class AFd 
{
protected:
	int fd;
	string type;
	size_t totalClean;
public:
	size_t maxToClean;
	static char *buff;
	bool deleteNow;
	AFd(int fd, string type);
	virtual void Handle() = 0;
	operator int() const ;
	string GetType();
	int GetFd();
	bool MarkedToDelete;
	bool cleanBody;
	virtual ~AFd();
	void cleanFd();
};