#pragma once
class AFd;

class IContext
{

public:
	virtual void Handle(AFd *fd) = 0;
};