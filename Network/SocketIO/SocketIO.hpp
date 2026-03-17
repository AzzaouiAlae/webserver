#pragma once

#include "Headers.hpp"
#include "Routing.hpp"
#define CLOSE_TIME 10
#define TIMEOUT 20

using namespace std;

enum IOState
{
	ePipe1 = 1,
	ePipe0 = 2,
	eSocket = 4,
};

enum IOError
{
	eSuccess = 0,
	eReadError = 1,
	eWriteError = 2,
	ePipe0Error = 3,
	ePipe1Error = 4,
	ePipeCreateError = 5,
};

enum TimeoutStatus
{
	eNotTimedOut = 0,
	eTimedOut = 1,
	eMarkedForDeletion = 2,
};

#define KBYTE 1024 * 64

class SocketIO : public ISocket
{
	static vector<pair<int, int> > pipePool;
	bool pipeInitialized;
	int pendingInPipe;
	int status;
	int SendedBuffToPipe;
	char *buff;
	time_t lastTime;
	int _timeoutStatus;
	int _timeout;
	bool _isKeepAlive;

public:
	int GetTimeoutStatus();
	bool isTimeOut();
	void UpdateTime();
	time_t GetEndTime() const;
	static void clearTimeout();
	static void closeTheOldestSocket();
	bool closeConnection;
private:
	struct TimeoutEntry
	{
		SocketIO *sock;
		time_t snapshotTime;

		bool operator>(const TimeoutEntry &other) const
		{
			return snapshotTime > other.snapshotTime;
		}
	};
	static priority_queue<TimeoutEntry, vector<TimeoutEntry>, greater<TimeoutEntry> > timeoutList;
	bool clearPipe();

public:
	void SetStateByFd(int fd);
	bool CanUsePipe0();
	bool CanUsePipe1();
	int pipefd[2];
	int Send(void *buff, int size);
	int SendBuffToPipe(void *buff, int size, bool usePending);
	int SendPipeToSock(int inputfd, size_t size = KBYTE);
	int SendSocketToPipe(int size, bool usePending);
	int SocketToFile(int fileFD, int size);
	int PipeToFile(int fileFD, int inputfd, int size);
	ssize_t FileToSocket(int fileFd, int size);
	int errorNumber;
	SocketIO(int fd, int timeout = TIMEOUT);
	void setKeepAlive(bool val);
	bool isKeepAlive() const;
	void setTimeout(int timeout);
	int getTimeout() const;
	~SocketIO();
	void Handle();
	void setupPipes();
};
