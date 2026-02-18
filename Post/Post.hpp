#pragma once
#include "../Headers.hpp"
#include "../AMethod/AMethod.hpp"


class Post : public AMethod
{
	int uploadFd;
	bool readyToUpload;
	bool pathResolved;
	size_t contentBodySize;
	size_t uploadedSize;
	char *buf;

	// ──── Single-responsibility helpers ────
	void ResolvePath();
	void OpenUploadFile();
	void WriteBodyFromMemory();
	void WriteBodyFromSocket();
	void uploadFileToDisk();

	// ──── Response helpers ────
	bool GetLocationReturn(string &retCode, string &retBody);
	void SendPostRedirection(const string &retCode, const string &retBody);
	void SendPostCustomBody(const string &retCode, const string &retBody);
	void SendPostDefault();
	void createPostResponse();
	void PostMethod();

public:
	Post(SocketIO *sock, Routing *router);

	~Post();
	bool HandleResponse();
};
