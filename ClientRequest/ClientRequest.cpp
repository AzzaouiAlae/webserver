/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientRequest.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aazzaoui <aazzaoui@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 20:05:46 by oel-bann          #+#    #+#             */
/*   Updated: 2026/02/20 18:13:13 by aazzaoui         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ClientRequest.hpp"

// GET /index.php HTTP/1.1
// Host: localhost:8080
// Connection: keep-alive
// Cache-Control: max-age=0
// sec-ch-ua: "Chromium";v="142", "Google Chrome";v="142", "Not_A Brand";v="99"
// sec-ch-ua-mobile: ?0
// sec-ch-ua-platform: "Linux"
// Upgrade-Insecure-Requests: 1
// User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/142.0.0.0 Safari/537.36
// Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
// Sec-Fetch-Site: none
// Sec-Fetch-Mode: navigate
// Sec-Fetch-User: ?1
// Sec-Fetch-Dest: document
// Accept-Encoding: gzip, deflate, br, zstd
// Accept-Language: fr-FR,fr;q=0.9,en-US;q=0.8,en;q=0.7

map<string, string> ClientRequest::_clientreqDirectives;

void ClientRequest::initReqDirectives()
{
	if (_clientreqDirectives.size() != 0)
		return;
	_clientreqDirectives["Content-Type"] = "CONTENT_TYPE";
	_clientreqDirectives["Content-Length"] = "CONTENT_LENGTH";

	_clientreqDirectives["Host"] = "HTTP_HOST";
	_clientreqDirectives["User-Agent"] = "HTTP_USER_AGENT";
	_clientreqDirectives["Accept"] = "HTTP_ACCEPT";
	_clientreqDirectives["Accept-Language"] = "HTTP_ACCEPT_LANGUAGE";
	_clientreqDirectives["Accept-Encoding"] = "HTTP_ACCEPT_ENCODING";
	_clientreqDirectives["Connection"] = "HTTP_CONNECTION";
	_clientreqDirectives["Cookie"] = "HTTP_COOKIE";
	_clientreqDirectives["Referer"] = "HTTP_REFERER";
	_clientreqDirectives["Origin"] = "HTTP_ORIGIN";
}

ClientRequest::ClientRequest(size_t maxbodysze) : ARequest()
{
	initReqDirectives();
	
	_Parspos = eParsStart;
	_maxbodysize = maxbodysze;
	_env["SERVER_SOFTWARE"] = "webserv/1.0";
	_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	_reqDirectives = &_clientreqDirectives;
	DEBUG("ClientRequest") << "ClientRequest initialized with maxBodySize=" << maxbodysze;
}

ClientRequest::ClientRequest() : ARequest()
{
	initReqDirectives();
	
	_Parspos = eParsStart;
	_maxbodysize = UINT64_MAX;
	_env["SERVER_SOFTWARE"] = "webserv/1.0";
	_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	_reqDirectives = &_clientreqDirectives;
	DEBUG("ClientRequest") << "ClientRequest initialized with default maxBodySize.";
}

ClientRequest::~ClientRequest() {}

string &ClientRequest::getPath() 
{
	return _env["REQUEST_URI"];
}

bool ClientRequest::parsPath(string path)
{
	size_t pos = path.find('?');

	if (path[0] != '/')
		return (false);
	_env["REQUEST_URI"] = path;
	if (pos != string::npos)
	{
		_env["SCRIPT_NAME"] = path.substr(0, pos);
		_env["QUERY_STRING"] = path.substr(pos + 1);
		DDEBUG("ClientRequest") << "parsPath: SCRIPT_NAME='" << _env["SCRIPT_NAME"]
								<< "', QUERY_STRING='" << _env["QUERY_STRING"] << "'";
	}
	else
	{
		_env["SCRIPT_NAME"] = path;
		DDEBUG("ClientRequest") << "parsPath: SCRIPT_NAME='" << _env["SCRIPT_NAME"] << "' (no query string)";
	}
	return (true);
}

void ClientRequest::parsHttpStandard(string httpStandard)
{
	stringstream ss(httpStandard);
	string method, path, httpv;
	string extra;

	if (!(ss >> method >> path >> httpv) || (ss >> extra))
		Error::ThrowError("The Request HttpStandard is Invalid");
	if (!(method == "GET" || method == "POST" || method == "DELETE"))
		Error::ThrowError("501");
	if (!(httpv == "HTTP/1.1" || httpv == "HTTP/1.0"))
		Error::ThrowError("The Request Protocol version Invalid");
	if (!parsPath(path))
		Error::ThrowError("The Request Path Invalid");
	_env["REQUEST_METHOD"] = method;
	_env["SERVER_PROTOCOL"] = httpv;
	_Parspos = eParsHttpStand;
	DEBUG("ClientRequest") << "Request line parsed: " << method << " " << path << " " << httpv;
}

void ClientRequest::parsLenTypeCont()
{
	if (_env.find("CONTENT_LENGTH") != _env.end() && _env.find("CONTENT_TYPE") == _env.end())
		Error::ThrowError("There is No Content Type");
	if (_env.find("CONTENT_TYPE") != _env.end() && _env.find("CONTENT_LENGTH") == _env.end())
		Error::ThrowError("There is No Content Length");

	if (_env.find("CONTENT_TYPE") != _env.end() && _env.find("CONTENT_LENGTH") != _env.end())
	{
		if (!getthereisbody())
			Error::ThrowError("400");
		if (!Utility::strtosize_t(_env["CONTENT_LENGTH"], _content_len))
			Error::ThrowError("The Content Length Invalid");
		DDEBUG("ClientRequest") << "parsLenTypeCont: Content-Type='" << _env["CONTENT_TYPE"]
								<< "', Content-Length=" << _content_len;
	}
}

void ClientRequest::parseHost()
{
	if (_env.find("Host") != _env.end())
	{
		Parsing::parseListen(_env["Host"], _env["SERVER_PORT"], _env["SERVER_NAME"]);
		DDEBUG("ClientRequest") << "parseHost: SERVER_NAME='" << _env["SERVER_NAME"]
								<< "', SERVER_PORT='" << _env["SERVER_PORT"] << "'";
	}
	else
		Error::ThrowError("Bad Request");	
}

bool ClientRequest::ParseHeader()
{
	string line = "";

	if (_Parspos == eParsStart && getFullLine(line))
		parsHttpStandard(line);
	while (_Parspos == eParsHttpStand && getFullLine(line) && line != "\r\n")
		parseHeaderLine(line);
	if (_Parspos == eParsHttpStand && line == "\r\n" && _requestbuff.length() > 0) _Thereisbody = true;
	if (_Parspos == eParsHttpStand && line == "\r\n" && 
		(_env["REQUEST_METHOD"] == "POST" || _env["REQUEST_METHOD"] == "DELETE"))
		parsLenTypeCont();
	if (_Parspos == eParsHttpStand && line == "\r\n")
	{
		_Parspos = eParsEnd;
		DEBUG("ClientRequest") << "Request header parsing complete: method=" << _env["REQUEST_METHOD"]
							   << ", path=" << _env["REQUEST_URI"]
							   << ", host=" << _env["HTTP_HOST"];
		return (true);
	}
	return (false);
}

bool ClientRequest::isRequestHeaderComplete()
{
	return _Parspos == eParsEnd;
}

bool ClientRequest::isComplete(char *request, int size)
{
	_requestbuff.append(request, size);
	DDEBUG("ClientRequest") << "isComplete: appended " << size << " bytes, total buffer=" << _requestbuff.length();
	if (_Parspos != eParsEnd)
	{
		if (!ParseHeader())
			return (false);
	}
	return true;
}


const string &ClientRequest::getMethod() const
{
	static const string empty = "";

	map<string, string>::const_iterator it =
		_env.find("REQUEST_METHOD");
	return (it != _env.end()) ? it->second : empty;
}

const string &ClientRequest::getHost() const
{
	static const string empty = "";

	map<string, string>::const_iterator it =
		_env.find("HTTP_HOST");

	return (it != _env.end()) ? it->second : empty;
}

const string &ClientRequest::getServerName() const
{
	static const string empty = "";

	map<string, string>::const_iterator it =
		_env.find("SERVER_NAME");

	return (it != _env.end()) ? it->second : empty;
}

const string &ClientRequest::getport() const
{
	static const string empty = "";

	map<string, string>::const_iterator it =
		_env.find("SERVER_PORT");

	return (it != _env.end()) ? it->second : empty;
}

void ClientRequest::setUrlPart(string scriptpath, string pathinfo)
{
	_env["SCRIPT_NAME"] = scriptpath;
	_env["PATH_INFO"] = pathinfo;
}

void ClientRequest::SetMaxBodySize(int size)
{
	_maxbodysize = size;
	DDEBUG("ClientRequest") << "SetMaxBodySize: maxBodySize=" << _maxbodysize
							<< ", bufferLen=" << _requestbuff.length()
							<< ", contentLen=" << _content_len;
	
	if (_requestbuff.length() > _maxbodysize || _content_len > _maxbodysize){
		Error::ThrowError("413");
	}
}
