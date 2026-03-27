#pragma once
#include "Headers.hpp"
#include "AMethod.hpp"

class Post : public AMethod
{
	enum PostStatus {
		eInitPost = 0,
		eUploadFile = 3,
		eMultipartUpload = 4,
		eChunkedUpload = 5,
	};
	int _uploadFd;
	size_t _contentBodySize;
	size_t _uploadedSize;
	Path *_path;
	originalPath *_originalPath;
	char *_buff;

	void _openUploadFile();
	void _writeBodyFromMemory();
	void _writeBodyFromSocket();
	void _uploadFileToDisk();

	bool _getLocationReturn(string &retCode, string &retBody);
	void _sendPostRedirection(const string &retCode, const string &retBody);
	void _sendPostCustomBody(const string &retCode, const string &retBody);
	void _createPostResponse();

	void _handleMultipartUpload();
	void _uploadChunkedToDisk();
	void _checkMaxBodySize(size_t bodySize);
	int _initMultipartUpload();
	void _initPost();
	void _decodBody(string &body, ChunkedData &decoder, int &status);
	ssize_t _uploadToDisk(int fd, char *data, ssize_t bodySize);
	void _uploadDataForm(FormData *formData);
public:
	Post(SocketIO *sock, Routing *router);
	~Post();
	bool HandleResponse();
};
