#pragma once
#include "AMethod.hpp"

class GET : public AMethod
{
	enum GETStatus {
		eSendAutoIndex = 3,
	};

	stringstream _filesList;
	string _filesListStr;
	int _sendListFiles;
	int _targetToSend;
	int _sent;

	void _openFile(const string &path);
	void _prepareFileResponse();
	void _serveFile();

	void _getStaticIndex();

	string _formatDirectoryEntry(const string &name, const struct stat &st, const string &requestPath);
	void _listFiles(const string &path);
	int  _calculateAutoIndexSize();
	void _createListFilesResponse();

	void _sendChunk(const char *data, int dataSize);
	void _sendListFilesStr(const string &str);
	void _sendAutoIndex(StaticFile *f);
	void _advanceListFilesState();
	void _sendListFilesResponse();

	void _createResponse();
public:
	GET(SocketIO *sock, Routing *router);
	~GET();

	bool HandleResponse();
};
