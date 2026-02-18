#pragma once 

#include "../Headers.hpp"
#include "../Routing/Routing.hpp"
#define CLOSE_TIME 10

using namespace std;

enum IOState {
	ePipe1 = 1,
	ePipe0 = 2,
	eSocket = 4,
};

#define KBYTE 1024
#define MBYTE 50

class SocketIO : public ISocket {
	static long CurrentTime();
	static vector<pair<int, int> > pipePool;
	bool pipeInitialized;
	int pendingInPipe;
	int status;
	int SendedBuffToPipe;
	char *buff;
public:
	int SendBuffToPipe(void *buff, int size);
	int SendPipeToSock();
	static int GetPipePoolSize();
	int pipefd[2];
	SocketIO(int fd);
    ~SocketIO();
	static int errorNumber;
	void SetStateByFd(int fd);
	int Send(void *buff, int size);
	ssize_t sendFileWithHeader(const char *httpHeader, int headerLen, int fileFd, int size);
	ssize_t FileToSocket(int fileFd, int size);
	int SocketToFile(int fileFD, int size);
	int SocketToSocketRead(int socket, int size);
	int SocketToSocketWrite(int socket, int size);
	void Handle();
	static int CloseSockFD(int fd);
	static void ClearPipePool();
	int SendSocketToPipe(int size = KBYTE * MBYTE);
};