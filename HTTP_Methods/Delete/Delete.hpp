#pragma once
#include "Headers.hpp"
#include "AMethod.hpp"

class Delete: public AMethod
{
	void _deleteFile();
	void _initDelete();
public:
	bool HandleResponse();
	Delete(ClientSocket *sock, Routing *router);
};
