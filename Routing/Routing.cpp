#include "Routing.hpp"

void Routing::CreatePath()
{
	AST<std::string> *loc;
	AST<std::string> *srv;
	try {
		srv = &Parsing::GetServerByHost(req->host, req->port);
		loc = &Parsing::GetLocationByPath(*srv, req->path);
	} catch (exception &e) {
		return;
	}
	string rootPtah = Parsing::GetRoot(*loc);
	if (rootPtah == "")
		rootPtah = Parsing::GetRoot(*srv);

	string index = Parsing::GetIndex(*loc);
	if (index == "")
		index = Parsing::GetIndex(*srv);

	
	if (Utility::GetFileExtension(req->path) == "")
		path = rootPtah +  "/" + req->path + "/" + index;
	else 
		path = rootPtah +  "/" + req->path;
	created = true;
}

int Routing::SendResponse(int sock, int size)
{
	int len = 0;
	if (header != "")
	{
		len = send(sock, header.c_str(), header.size(), O_NONBLOCK);
		header = "";
		size -= len;
		if (size <= 0)
			return len;
	}
	len += sendfile(sock, pathFD, 0, size);
	return len;
}

void Routing::CreadHeader(string type, int size)
{
	stringstream ss;
	
	ss << "HTTP/1.1 200 OK\r\n"
		"Content-Type: " << type << "\r\n" <<
		"Content-Length: " << size << "\r\n" <<
		"Connection: close\r\n"
		"\r\n";

	header = ss.str();
}

void Routing::CreateResponse(request& req)
{
	this->req = &req;
	CreatePath();
	if (req.method == "GET")
	{
		if (created)
		{
			pathFD = open(path.c_str(), O_RDONLY | O_NONBLOCK);
			int size = Utility::getFileSize(path);
			string extension = Utility::GetFileExtension(path);
			string type = (Singleton::GetMime())[extension];
			CreadHeader(type, size);
			isReadyToSend = true;
		}
	}
}

bool Routing::isResponseSend()
{
	return sended;
}

Routing::Routing()
{
	sended = false;
	isReadyToSend = false;
	created = false;
}

bool Routing::isCreated()
{
	return created;
}

bool Routing::ReadyToSend()
{
	return isReadyToSend;
}