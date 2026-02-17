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

	// ──── Single-responsibility helpers ────
	void ResolvePath();
	void OpenUploadFile();
	void WriteBodyFromMemory();
	void WriteBodyFromSocket();
	void uploadFileToDisk();
	void createPostResponse();
	void PostMethod();

public:
	Post(SocketIO *sock, Routing *router);
	~Post();
	bool HandleResponse();
};