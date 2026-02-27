#pragma once
#include "Headers.hpp"

class ConnectionContext : public IContext
{
public:
	vector<Config::Server > *servers;
	ConnectionContext();
	~ConnectionContext();
	void Handle(AFd *fd);
};
