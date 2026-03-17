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
	AFd(int fd, string type);
	virtual ~AFd();
	virtual void Handle() = 0;
	static char *buff;
	operator int() const;
	string GetType();
	int GetFd();
	bool MarkedToDelete;
	bool cleanBody;
	void cleanFd();
};