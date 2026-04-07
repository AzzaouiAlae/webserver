#pragma once
#include <iostream>
using namespace std;

class AFd
{
protected:
	int fd;
	string type;
	size_t totalClean;
	unsigned int events;

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
	void SetEvents(unsigned int ev);
	unsigned int GetEvents() const;
};