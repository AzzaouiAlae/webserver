#pragma once
#include "../Headers.hpp"
#include "../AMethod/AMethod.hpp"

class Delete: AMethod
{
	void deleteFile();
	Config::Server::Location *loc;
public:
	bool HandleResponse();
};