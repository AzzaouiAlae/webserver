#pragma once

#include "GET.hpp"
#include "Post.hpp"
#include "Delete.hpp"


class Repsense
{
	enum Status {
		eCreateMethod,
		eHandleResponse,
	};
	AMethod *_method;
	SocketIO *_sock;
	Routing *_router;
	Repsense::Status _status;
	void _createMethod();
public:
	Repsense();
	~Repsense();

	void Init(SocketIO *sock, Routing *router);
	bool HandleResponse();
	void HandelErrorPages(const string &err);
};