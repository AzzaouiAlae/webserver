#pragma once
#include "Headers.hpp"
#include "Routing.hpp"
#include "ClientSocket.hpp"

using namespace std;

class NetIO
{
	struct TimeoutEntry
	{
		ClientSocket *sock;
		time_t snapshotTime;

		bool operator>(const TimeoutEntry &other) const
		{
			return snapshotTime > other.snapshotTime;
		}
	};
	static priority_queue<TimeoutEntry, vector<TimeoutEntry>, greater<TimeoutEntry> > timeoutList;
	
	NetIO();
	~NetIO();
public:
	enum TimeoutStatus
	{
		eNotTimedOut = 0,
		eTimedOut = 1,
		eSendTimeout = 2,
		eMarkedForDeletion = 3,
	};
	static long GetSocketTimeout();
	static void ClearIncorrectTimeout();
	static void ClearTimeout();
	static void CloseTheOldestSocket();
	static int Send(void *buff, int size, int fd);
	static int BuffToPipe(void *buff, int size, int pipefd);
	static int PipeToSock(int inputfd, size_t size, int socketFd);
	static int SocketToPipe(int size, int socketFd, int pipefd);
	static int PipeToFile(int fileFD, int inputfd, int size);
	static ssize_t FileToSocket(int fileFd, int size, int socketFd);
	static void AddToTimeoutList(ClientSocket *sock);
};
