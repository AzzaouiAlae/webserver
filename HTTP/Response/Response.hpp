#pragma once
// #include "AMethod.hpp"
#include "GET.hpp"
#include "Post.hpp"
#include "Delete.hpp"

class Repsense
{
	AMethod *method;
	SocketIO *sock;
	Routing *router;

public:
	Repsense();
	~Repsense();

	void Init(SocketIO *sock, Routing *router);
	bool HandleResponse();
	void HandelErrorPages(const string &err);
};