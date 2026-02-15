#pragma once
#include "../Headers.hpp"
#include "../SocketIO/SocketIO.hpp"
#include "../Socket/Socket.hpp"


class Repsense 
{
	stringstream responseHeader;
	string responseHeaderStr;
	StaticFile *staticFile;
	bool readyToSend;
	string filename;
	bool sendHeader;
	int ShouldSend;
	int filesize;
	string code;
	int sended;
	int fileFd;
	bool del;
	stringstream filesList;

	SocketIO *sock;
	vector<AST<string> > *servers;
public:
	Repsense();
	void Init(SocketIO *sock, vector<AST<string> > *servers);
	string CreateDate();
	void SendGetResponse();
	bool HandleResponse();
	void CreateResponseHeader();
	void GetMethod();
	void HandelErrorPages(const string& err);
	~Repsense();
	void GetStaticIndex();
	void listFiles(string path);
	void CreateListFilesRepsense();
};