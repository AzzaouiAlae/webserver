#pragma once
#include "Headers.hpp"
#include "AMethod.hpp"

class Post : public AMethod
{
	Path *_path;
	originalPath *_originalPath;

	bool _getLocationReturn(string &retCode, string &retBody);
	void _createPostRedirection(const string &retCode, const string &retBody);
	void _createPostCustomBody(const string &retCode, const string &retBody);
	void _createPostResponse();
	void _initPost();
public:
	Post(ClientSocket *sock, Routing *router);
	~Post();
	bool HandleResponse();
};
