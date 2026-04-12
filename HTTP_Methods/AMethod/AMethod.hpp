#pragma once
#include "Headers.hpp"
#include "NetIO.hpp"
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
	string _createUpdatedFilesJSONBody(set<string> &filesUploaded);

protected:
	enum Status {
		eInit = 0,
		eSendResponse = 1,
		eCGIResponse = 2,
		eUploadFile = 3,
		eSendAutoIndex = 4,
		eComplete = 5,
	};
	int _status;

	Routing *_router;
	ClientSocket *_sock;
	Cgi	*_cgi;
	Multiplexer *_multiplexer;

	string _filename;
	int _fileFd;
	StaticFile *_staticFile;

	string _statusCode;
	string _responseHeaderStr;
	stringstream _responseHeader;

	vector<pair<char *, size_t> > _buffers;
	
	size_t _bodySize;

	void _createResponseHeader(const string &body);
	void _createRedirectionHeader(const string &redirCode, const string &redirLocation);
	void _createTimeoutResponse();
	void _createRedirection();
	void _createDefaultResponse(const string &code);

	bool _isMethodAllowed(const string &method);
	void _resolvePath();
	string _escapeForJS(const string& input);
	void _handleCGI();

	void _addPair(StaticFile *f);
	void _addPair(string &str);
	void _insertPair(string &str, int idx = 0);
	void _addPair(const string &str);
	void _addPair(const char *data, size_t size);
	void _addPair(char *data, size_t size);
	void _addPair(pair<char *, size_t> &data);
	void _handleStrategyStatus(AStrategy *strategy);
	void _executeStrategy(AStrategy *strategy);
public:
	AMethod(ClientSocket *sock, Routing *router);
	virtual ~AMethod();
	virtual bool HandleResponse() = 0;
	void   HandleErrorPages(const string &err);
	static map<string, string>& getStatusMap();
};