#pragma once
// #include "AMethod.hpp"
#include "../GET/GET.hpp"
#include "../Post/Post.hpp"
// #include "DELETE.hpp"

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