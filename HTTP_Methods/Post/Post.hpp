#pragma once
#include "Headers.hpp"
#include "AMethod.hpp"

class Post : public AMethod
{
	int uploadFd;
	bool readyToUpload;
	size_t contentBodySize;
	size_t uploadedSize;

	void OpenUploadFile();
	void WriteBodyFromMemory();
	void WriteBodyFromSocket();
	void uploadFileToDisk();

	bool GetLocationReturn(string &retCode, string &retBody);
	void SendPostRedirection(const string &retCode, const string &retBody);
	void SendPostCustomBody(const string &retCode, const string &retBody);
	void createPostResponse();
	void PostMethod();

public:
	Post(SocketIO *sock, Routing *router);
	~Post();
	bool HandleResponse();
};
