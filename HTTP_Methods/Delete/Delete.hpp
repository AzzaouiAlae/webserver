#pragma once
#include "Headers.hpp"
#include "AMethod.hpp"

class Delete: public AMethod
{
	enum Status {
		eInit = 0,
	};
	void _deleteFile();
	void _initDelete();
public:
	bool HandleResponse();
	Delete(SocketIO *sock, Routing *router);
};
