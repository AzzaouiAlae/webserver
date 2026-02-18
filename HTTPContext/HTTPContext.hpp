#pragma once
#include "../Headers.hpp"
#include "../Socket/Socket.hpp"
#include "../SocketIO/SocketIO.hpp"
#include "../Pipe/Pipe.hpp"
#include "../Repsense/Repsense.hpp"

#define BUF_SIZE (1024 * 1024 * 10)

class HTTPContext : public IContext
{
	Repsense repsense;
	Routing router;
	SocketIO *sock;
	char *buf;
	AFd *out;
	AFd *in;
	bool err;
	string errNo;
	bool isMaxBodyInit;

	// Helper to handle buffer allocation and raw socket reading
    int  _readFromSocket();

    // Helper to parse the buffer and find the correct Server block
    bool _parseAndConfig();

    // Helper to set up Pipes and switch Epoll state when request is done
    void _setupPipeline();
public:
	void activeInPipe();
	void activeOutPipe();
	HTTPContext();
	~HTTPContext();
	vector<Config::Server > *servers;
	void Handle(AFd *fd);
	void Handle(Socket *sock);
	void Handle();
	void HandleRequest();
	void MarkedSocketToFree();
	void setMaxBodySize();
	void addInEvent(void (*f)(AFd *fd));
	void addOutEvent(void (*f)(AFd *fd));

};