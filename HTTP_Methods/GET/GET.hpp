#pragma once
#include "AMethod.hpp"

class GET : public AMethod
{
	stringstream _filesList;
	string _filesListStr;

	void _openFile(const string &path);
	void _prepareFileResponse();
	void _serveFile();

	string _formatDirectoryEntry(const string &name, const struct stat &st, const string &requestPath);
	void _listFiles(const string &path);
	int  _calculateAutoIndexSize();
	void _createListFilesResponse();

	void _createResponse();
public:
	GET(ClientSocket *sock, Routing *router);
	~GET();

	bool HandleResponse();
};
