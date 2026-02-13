#pragma once
#include "../Headers.hpp"
#include "../Socket/Socket.hpp"
#include "../SocketIO/SocketIO.hpp"
#include "../Pipe/Pipe.hpp"

#define BUF_SIZE (1024 * 1024 * 10)

class HTTPContext : public IContext
{
	int fileFd;
	bool readyToSend;
	bool sendHeader;
	stringstream responseHeader;
	int filesize;
	string filename;
	int sended;
	AFd *in;
	AFd *out;
	SocketIO *sock;
	string responseHeaderStr;
	int ShouldSend;
	char *buf;
public:
	void activeInPipe();
	void activeOutPipe();
	HTTPContext();
	~HTTPContext();
	vector<AST<string> > *servers;
	void Handle(AFd *fd);
	void Handle(Socket *sock);
	void Handle();
	void HandleRequest();
	void HandleResponse();
	void GetMethod();
	void CreateResponseHeader();
	string GetStatusCode();
	string CreateDate();
	void SendGetResponse();
	void MarkedSocketToFree();
};