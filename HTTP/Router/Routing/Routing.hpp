#pragma once
#include "Headers.hpp"
#include <sys/sendfile.h>

class Routing 
{
	Path _path;
	string _strPath;
	ClientRequest _request;
	ChunkedData _chunkedData;
	MultipartData _multipartData;
	AStrategy *_sendStrategy;
	AStrategy *_readStrategy;
public:
	Config::Server *srv;
	Config::Server::Location *loc;
	ClientRequest &GetRequest();
	Path &GetPath();
	Routing();
	~Routing();
	string CreatePath(Config::Server *srv);
	ChunkedData &GetChunkedData();
	MultipartData &GetMultipartData();
	void AddPath(string &filename);
	void SetSendStrategy(AStrategy *strategy);
	void SetReadStrategy(AStrategy *strategy);
	AStrategy *GetSendStrategy();
	AStrategy *GetReadStrategy();
};

