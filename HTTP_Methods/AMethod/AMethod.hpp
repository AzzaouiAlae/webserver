#pragma once
#include "Headers.hpp"
#include "SocketIO.hpp"
#include "Socket.hpp"
#include "SessionManager.hpp"
#include "Cgi.hpp"

class AMethod
{
	static map<string, string> statusMap;
	static void InitStatusMap();

	string ResolveServerName();
	string ResolveMimeType();

protected:
	stringstream responseHeader;
	string responseHeaderStr;
	StaticFile *staticFile;
	bool readyToSend;
	string filename;
	bool sendHeader;
	int ShouldSend;
	int bodySize;
	string code;
	int sended;
	int fileFd;
	bool del;
	Routing *router;
	SocketIO *sock;
	bool pathResolved;
	Config::Server::Location *loc;
	Cgi	*_cgi;
	string CreateDate();
	void CreateResponseHeader();
	void CreateRedirectionHeader(const string &redirCode, const string &redirLocation);

	string ResolveErrorFilePath(const string &errorCode);
	bool   OpenErrorFile(const string &path);
	void   LoadStaticErrorFile(const string &errorCode);

	void SendResponse();
	void SendRedirection();
	void SendDefaultRespense(const string &code);

	bool IsMethodAllowed(const string &method);

	void ResolvePath();

	string escapeForJS(const string& input);
	void addCookies(Session &session);
	
	void HandelCGI();
	void addAlowMethodsHeader();
	void addCookiesHeader();
	
public:
	
	AMethod(SocketIO *sock, Routing *router);
	virtual ~AMethod();
	virtual bool HandleResponse() = 0;
	void   HandelErrorPages(const string &err);
	static map<string, string>& getStatusMap();
};