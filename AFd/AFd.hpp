#pragma once
#include <iostream>
using namespace std;


class AFd 
{
protected:
	int fd;
	string type;
public:
	bool deleteNow;
	AFd(int fd, string type);
	virtual void Handle() = 0;
	operator int() const ;
	string GetType();
	int GetFd();
	bool MarkedToDelete;
	virtual ~AFd();
};