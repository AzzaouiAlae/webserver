#pragma once

#include "../../Headers.hpp"

class AFd ;

class ISocket : public AFd 
{
public:
	IContext *context;
	ISocket(int fd, string type);
};
