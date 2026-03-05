#include "APipe.hpp"

vector<pair<int, int> > APipe::pipePool;

APipe::APipe(int fd, string type): AFd(fd, type)
{}

void APipe::ClearPipePool()
{
	for (int i = 0; i < (int)pipePool.size(); i++)
	{
		close(pipePool[i].first);
		close(pipePool[i].second);
	}
}

int APipe::GetPipePoolSize()
{
	return pipePool.size();
}

bool APipe::GetPipe(int pipe[2])
{
	if (pipePool.size() > 0)
	{
		pair<int, int> p = pipePool[pipePool.size() - 1];
		pipe[0] = p.first;
		pipe[1] = p.second;
		pipePool.pop_back();
		DDEBUG("SocketIO") <<"reused pipe from pool [" << pipe[0] << ", " << pipe[1] << "]";
	}
	else if (pipe2(pipe, O_NONBLOCK | O_CLOEXEC) == -1)
	{
		ERR() << "SocketIO created pipe failed!";
		return false;
	}
	DEBUG("SocketIO") << "SocketIO pipe created [" << pipe[0] << ", " << pipe[1] << "]";
	return true;
}

void APipe::ReleasePipe(int pipe[2])
{
	if (pipePool.size() < 100) {
		pipePool.push_back(pair<int, int>(pipe[0], pipe[1]));
	}
	else {
		close(pipe[0]);
		close(pipe[1]);
	}
}