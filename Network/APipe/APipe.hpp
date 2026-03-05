#pragma once
#include "Headers.hpp"

class APipe: public AFd
{
	static vector<pair<int, int> > pipePool;
public:
	static int GetPipePoolSize();
	static void ClearPipePool();
	static bool GetPipe(int pipe[2]);
	static void ReleasePipe(int pipe[2]);
	APipe(int fd, string type);
	virtual ~APipe() {}
};