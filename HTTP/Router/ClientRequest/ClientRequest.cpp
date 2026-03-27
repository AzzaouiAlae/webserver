/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientRequest.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aazzaoui <aazzaoui@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 20:05:46 by oel-bann          #+#    #+#             */
/*   Updated: 2026/03/05 23:06:23 by aazzaoui         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ClientRequest.hpp"

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
	_clientreqDirectives["Transfer-Encoding"] = "HTTP_TRANSFER_ENCODING";
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
	return _env["SCRIPT_NAME"];
}

bool ClientRequest::parsPath(string path)
{
	size_t pos = path.find('?');

	if (path[0] != '/')
		return (false);
	Utility::normalizePath(path);
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
		Error::ThrowError("400");
	if (!(httpv == "HTTP/1.1" || httpv == "HTTP/1.0"))
		Error::ThrowError("505");
	if (!parsPath(path))
		Error::ThrowError("400");
	_env["REQUEST_METHOD"] = method;
	_env["SERVER_PROTOCOL"] = httpv;
	_Parspos = eParsHttpStand;
	DEBUG("ClientRequest") << "Request line parsed: " << method << " " << path << " " << httpv;
}

void ClientRequest::parsLenTypeCont()
{
	if (getthereisbody() || _env.find("Content-Length") != _env.end())
	{
		if (_isChunked)
			return;
		if (_env.find("CONTENT_LENGTH") != _env.end())
			if (!Utility::strtosize_t(_env["CONTENT_LENGTH"], _content_len))
				Error::ThrowError("400");
		if (_content_len == 0 || _env.find("CONTENT_LENGTH") == _env.end())
			Error::ThrowError("400");
		DDEBUG("ClientRequest") << "parsLenTypeCont: Content-Type='" << _env["CONTENT_TYPE"]
								<< "', Content-Length=" << _content_len;
		if (_content_len > _maxbodysize ){
			Error::ThrowError("413");
		}
	}
}

void ClientRequest::parseHost()
{
	if (_env.find("Host") != _env.end())
	{
		Config::parseListen(_env["Host"], _env["SERVER_PORT"], _env["SERVER_NAME"]);
		DDEBUG("ClientRequest") << "parseHost: SERVER_NAME='" << _env["SERVER_NAME"]
								<< "', SERVER_PORT='" << _env["SERVER_PORT"] << "'";
	}
	else
		Error::ThrowError("400");	
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
	{
		detectTransferEncoding();
		detectMultipartBoundary();
		parsLenTypeCont();
	}
	if (_Parspos == eParsHttpStand && line == "\r\n")
	{
		_Parspos = eParsEnd;
		DEBUG("ClientRequest") 
			<< "Request header parsing complete: method=" << _env["REQUEST_METHOD"]
			<< ", path=" << _env["REQUEST_URI"]
			<< ", host=" << _env["HTTP_HOST"];
		checkHost();
		return (true);
	}
	return (false);
}

void ClientRequest::checkHost()
{
	if (_env["SERVER_PROTOCOL"] != "HTTP/1.1")
		return;
	if (_env["HTTP_HOST"] == "")
		Error::ThrowError("400");
}

bool ClientRequest::isRequestHeaderComplete()
{
	return _Parspos == eParsEnd;
}

bool ClientRequest::isComplete(char *request, int size)
{
	_requestbuff.append(request, size);
	DDEBUG("ClientRequest") 
		<< "isComplete: appended " << size 
		<< " bytes, total buffer=" << _requestbuff.length() 
		<< "\n" << _requestbuff;
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
	
	if (_content_len > _maxbodysize ){
		Error::ThrowError("413");
	}
}

bool  ClientRequest::getFullLine(string &line)
{
	int index = 0;
	line.clear();

	while (_requestbuff[index] && _requestbuff[index] != '\n')
	{
		if (index > MAXHEADERSIZE)
        {
            if (_firstline)
                Error::ThrowError("414");
            Error::ThrowError("431");
        }
		line += _requestbuff[index];
		index++;
	}
	DDEBUG("ClientRequest") << "getFullLine: extracted line='" << line << "' from buffer, index=" << index;	
    _firstline = false;
	if (_requestbuff[index] == '\n' && (index == 0 || (index > 0 && _requestbuff[index - 1] != '\r') ))
        Error::ThrowError("400");
	else if (_requestbuff[index] == '\n')
	{
		_headersize += index;
        if (_headersize > MAXHEADERSIZE)
            Error::ThrowError("431");
		_requestbuff.erase(0, index + 1);
		line += '\n';
	}
	else
	{
		line.clear();
		return (false);
	}
	return true;
}

void ClientRequest::parseHeaderLine(const string &line)
{
    size_t colonPos = line.find(':');
    if (colonPos == string::npos)
        Error::ThrowError("400");
    string key   = line.substr(0, colonPos);
    if (!key.empty() && !isalpha(key[key.size() - 1]))
        Error::ThrowError("400");
    string value = line.substr(colonPos + 1);
    if (value.size() >= 2 && key[0] == ' ' && !isalpha(key[1]))
        Error::ThrowError("400");
    Utility::trim(value, " \n\r");

    if (_reqDirectives && _reqDirectives->find(key) != _reqDirectives->end())
        key = (*_reqDirectives)[key];

    if (_env.find(key) != _env.end())
    {
        if (key != "Cookie")
            Error::ThrowError("400");
    }
    _env[key] = value;
}

void ClientRequest::detectTransferEncoding()
{
	if (_env.find("HTTP_TRANSFER_ENCODING") != _env.end()
		&& _env["HTTP_TRANSFER_ENCODING"] == "chunked")
	{
		_isChunked = true;
		_Thereisbody = true;
		DEBUG("ClientRequest") << "Detected chunked transfer encoding.";
	}
}

void ClientRequest::detectMultipartBoundary()
{
	if (isMultipartFormData())
	{
		_isMultipart = true;
		_boundary = getMultipartBoundary();
		if (_boundary.empty())
			Error::ThrowError("400");
		DEBUG("ClientRequest") << "Detected multipart/form-data, boundary='" << _boundary << "'";
	}
}

bool ClientRequest::isKeepAlive() const
{
	map<string, string>::const_iterator it = _env.find("HTTP_CONNECTION");
	if (it != _env.end())
	{
		string val = it->second;
		for (size_t i = 0; i < val.size(); i++)
			val[i] = tolower(val[i]);
		return val == "keep-alive";
	}
	map<string, string>::const_iterator proto = _env.find("SERVER_PROTOCOL");
	if (proto != _env.end() && proto->second == "HTTP/1.1")
		return true;
	return false;
}

size_t ClientRequest::getMaxBodySize() const
{
	return _maxbodysize;
}