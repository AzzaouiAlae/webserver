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

class SocketIO : public AFd {
	static long CurrentTime();
	static vector<pair<int, int> > pipePool;
	bool pipeInitialized;
	int pendingInPipe;
	int SendBuffToPipe(void *buff, int size);
	int SendPipeToSock();
	Routing router;
	int status;
	int SendedBuffToPipe;
	char *buff;
public:
	int pipefd[2];
	SocketIO(int fd);
    ~SocketIO();
	static int errorNumber;
	void SetStateByFd(int fd);
	int Send(void *buff, int size = 64 * 1024);
	ssize_t sendFileWithHeader(const char *httpHeader, int headerLen, int fileFd, int size = 64 * 1024);
	ssize_t FileToSocket(int fileFd, int size = 64 * 1024);
	int SocketToFile(int fileFD, int size = 64 * 1024);
	int SocketToSocketRead(int socket, int size = 64 * 1024);
	int SocketToSocketWrite(int socket, int size = 64 * 1024);
	void Handle();
	static int CloseSockFD(int fd);
	Routing &GetRouter();
	static void ClearPipePool();
};
