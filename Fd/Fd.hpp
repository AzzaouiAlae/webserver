#pragma once
#include "../Headers.hpp"

class Fd 
{
public:
	enum Types {
		eSocket,
		eClient,
		eOther
	};
	enum Status {
		eNone,
		eRequest,
		eResponse
	};
	int fd;
	Types type;
	Status status;
	long time;
	request req;
	Routing routing;
	
	Fd(int fd, Types type = eSocket, Status status = eNone);
	operator int() const;
	bool isTimeOut();
};