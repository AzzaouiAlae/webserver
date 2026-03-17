#pragma once
#include "Headers.hpp"
#include "AMethod.hpp"


class Post : public AMethod
{
	int uploadFd;
	bool readyToUpload;
	size_t contentBodySize;
	size_t uploadedSize;
	Path *_path;
	originalPath *_originalPath;
	char *_buff;

	void OpenUploadFile();
	void WriteBodyFromMemory();
	void WriteBodyFromSocket();
	void uploadFileToDisk();

	bool GetLocationReturn(string &retCode, string &retBody);
	void SendPostRedirection(const string &retCode, const string &retBody);
	void SendPostCustomBody(const string &retCode, const string &retBody);
	void createPostResponse();
	void PostMethod();

	// Multipart form-data support
	struct MultipartPart {
		map<string, string> headers;
		string filename;
		string name;
		string contentType;
		string body;
	};
	vector<MultipartPart> _parts;
	bool _isMultipartUpload;
	string _multipartBuffer;

	void handleMultipartUpload();
	bool parseMultipartBody();
	bool extractPartInfo(const string &headerSection, MultipartPart &part);
	void uploadMultipartParts();

	string resolveUploadFileName();
	string extractFilenameFromDisposition(const string &header);

	// Chunked transfer encoding support
	bool _isChunkedUpload;
	void uploadChunkedToDisk();

public:
	Post(SocketIO *sock, Routing *router);
	~Post();
	bool HandleResponse();
};
