#pragma once
#include "Headers.hpp"
#include "SocketIO.hpp"
#include "Socket.hpp"
#include "SessionManager.hpp"
#include "Cgi.hpp"

class AMethod
{
	static map<string, string> _statusMap;
	static void _initStatusMap();

	string _resolveServerName();
	string _resolveMimeType();
	string _resolveErrorFilePath(const string &errorCode);

	bool   _openErrorFile(const string &path);
	void   _loadStaticErrorFile(const string &errorCode);
	string _createDate();
	void _addCookies(Session &session);
	void _addAlowMethodsHeader();
	void _addCookiesHeader();

protected:
	enum Status {
		eCreateResponse = 0,
		eSendResponse = 1,
		eCGIResponse = 2,
		eComplete = 100,
	};
	int _status;

	Routing *_router;
	SocketIO *_sock;
	Config::Server::Location *_loc;
	Cgi	*_cgi;
	Multiplexer *_multiplexer;

	string _filename;
	int _fileFd;
	StaticFile *_staticFile;

	string _statusCode;
	string _responseHeaderStr;
	stringstream _responseHeader;

	IStrategy *_sendStrategy;
	vector<pair<char *, size_t> > _buffers;
	
	int _sended;
	int _bodySize;
	int _totalByteToSend;

	void _createResponseHeader();
	void _createRedirectionHeader(const string &redirCode, const string &redirLocation);
	void _createTimeoutResponse();

	void _sendResponse();
	void _sendRedirection();
	void _sendDefaultRespense(const string &code);

	bool _isMethodAllowed(const string &method);
	void _resolvePath();
	string _escapeForJS(const string& input);
	void _handelCGI();

public:
	AMethod(SocketIO *sock, Routing *router);
	virtual ~AMethod();
	virtual bool HandleResponse() = 0;
	void   HandelErrorPages(const string &err);
	static map<string, string>& getStatusMap();
};