#pragma once
#include "../Headers.hpp"
#include "../SocketIO/SocketIO.hpp"
#include "../Socket/Socket.hpp"

class AMethod
{
	// ──── Static ────
	static map<string, string> statusMap;
	static void InitStatusMap();

	// ──── Header building helpers (single responsibility each) ────
	string ResolveServerName();
	string ResolveMimeType();

protected:
	// ──── State shared by all methods ────
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

	// ──── Shared utilities (each does one thing) ────
	string CreateDate();
	void CreateResponseHeader();
	void CreateRedirectionHeader(const string &redirCode, const string &redirLocation);

	// ──── Error handling (broken into single-responsibility steps) ────
	string ResolveErrorFilePath(const string &errorCode);
	bool   OpenErrorFile(const string &path);
	void   LoadStaticErrorFile(const string &errorCode);

	// ──── Sending ────
	void SendResponse();
	void SendRedirection();

	// ──── Method validation ────
	bool IsMethodAllowed(const string &method);

public:
	AMethod(SocketIO *sock, Routing *router);
	virtual ~AMethod();

	// ──── Pure virtual: each HTTP method subclass must implement ────
	virtual bool HandleResponse() = 0;
	void   HandelErrorPages(const string &err);
	static map<string, string>& getStatusMap();
};