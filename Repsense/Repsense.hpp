#pragma once
#include "../Headers.hpp"
#include "../SocketIO/SocketIO.hpp"
#include "../Socket/Socket.hpp"


class Repsense 
{
	static map<string, string> statusMap;
	stringstream responseHeader;
	string responseHeaderStr;
	StaticFile *staticFile;
	bool readyToSend;
	string filename;
	bool sendHeader;
	int ShouldSend;
	int bodySize;
	string code;
	int sended;
	int fileFd;
	bool del;
	stringstream filesList;
	string filesListStr;
	int sendListFiles;
	int targetToSend;
	int sent;
	Routing *router;
	SocketIO *sock;
	static void InitStatusMap();

public:
	Repsense();
	void SendRedirection();
	void Init(SocketIO *sock, Routing *router);
	string CreateDate();
	void SendGetResponse();
	bool HandleResponse();
	void CreateResponseHeader();
	void GetMethod();
	void HandelErrorPages(const string& err);
	void SendListFilesStr(const string &str);
	~Repsense();
	void GetStaticIndex();
	void listFiles(string path);
	void CreateListFilesRepsense();
	void SendListFilesRepsense();
	void SendAutoIndex(StaticFile *f);
	void ServeFile();
};
