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
public:
	Config::Server *srv;
	Config::Server::Location *loc;
	ClientRequest &GetRequest();
	Path &GetPath();
	Routing();
	string CreatePath(Config::Server *srv);
	ChunkedData &GetChunkedData();
	MultipartData &GetMultipartData();
};

