#pragma once
#include "AMethod.hpp"

class GET : public AMethod
{
	stringstream filesList;
	string filesListStr;
	int sendListFiles;
	int targetToSend;
	int sent;
	void OpenFile(const string &path);
	void PrepareFileResponse();
	void ServeFile();

	void GetStaticIndex();

	string FormatDirectoryEntry(const string &name, const struct stat &st, const string &requestPath);
	void ListFiles(const string &path);
	int  CalculateAutoIndexSize();
	void CreateListFilesResponse();

	void SendChunk(const char *data, int dataSize);
	void SendListFilesStr(const string &str);
	void SendAutoIndex(StaticFile *f);
	void AdvanceListFilesState();
	void SendListFilesResponse();

	void GetMethod();
public:
	GET(SocketIO *sock, Routing *router);
	~GET();

	bool HandleResponse();
};