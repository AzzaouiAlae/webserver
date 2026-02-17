/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aazzaoui <aazzaoui@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 20:05:46 by oel-bann          #+#    #+#             */
/*   Updated: 2026/02/17 21:36:18 by aazzaoui         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

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

map<string, string> Request::_reqDirectives;

void Request::initReqDirectives()
{
	if (_reqDirectives.size() != 0)
		return;
	_reqDirectives["Content-Type"] = "CONTENT_TYPE";
	_reqDirectives["Content-Length"] = "CONTENT_LENGTH";

	_reqDirectives["Host"] = "HTTP_HOST";
	_reqDirectives["User-Agent"] = "HTTP_USER_AGENT";
	_reqDirectives["Accept"] = "HTTP_ACCEPT";
	_reqDirectives["Accept-Language"] = "HTTP_ACCEPT_LANGUAGE";
	_reqDirectives["Accept-Encoding"] = "HTTP_ACCEPT_ENCODING";
	_reqDirectives["Connection"] = "HTTP_CONNECTION";
	_reqDirectives["Cookie"] = "HTTP_COOKIE";
	_reqDirectives["Referer"] = "HTTP_REFERER";
	_reqDirectives["Origin"] = "HTTP_ORIGIN";
}

Request::Request(size_t maxbodysze)
{
	initReqDirectives();

	_Parspos = eParsStart;
	_requestbuff = "";
	_maxbodysize = maxbodysze;
	_env["SERVER_SOFTWARE"] = "webserv/1.0";
	_env["GATEWAY_INTERFACE"] = "CGI/1.1";
}

Request::Request()
{
	initReqDirectives();

	_maxbodysize = UINT64_MAX;
	_Parspos = eParsStart;
	_requestbuff = "";
	_env["SERVER_SOFTWARE"] = "webserv/1.0";
	_env["GATEWAY_INTERFACE"] = "CGI/1.1";
}

bool Request::getFullLine(string &line)
{
	int index = 0;
	line.clear();

	while (_requestbuff[index] && _requestbuff[index] != '\n')
	{
		if (index > MAXHEADERSIZE)
			Error::ThrowError("The Header is Bigger Than Expected");
		line += _requestbuff[index];
		index++;
	}
	if (_requestbuff[index] == '\n' && (index == 0 || (index > 0 && _requestbuff[index - 1] != '\r')))
		Error::ThrowError("The Request Line Invalid");
	else if (_requestbuff[index] == '\n')
	{
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

string &Request::getPath() 
{
	return _env["REQUEST_URI"];
}

bool Request::parsPath(string path)
{
	size_t pos = path.find('?');

	if (path[0] != '/')
		return (false);
	_env["REQUEST_URI"] = path;
	if (pos != string::npos)
	{
		_env["SCRIPT_NAME"] = path.substr(0, pos);
		_env["QUERY_STRING"] = path.substr(pos + 1);
	}
	else
		_env["SCRIPT_NAME"] = path;
	return (true);
}

void Request::parsHttpStandard(string httpStandard)
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
}

void Request::parsLenTypeCont()
{
	if (_env.find("CONTENT_LENGTH") != _env.end() && _env.find("CONTENT_TYPE") == _env.end())
		Error::ThrowError("There is No Content Type");
	if (_env.find("CONTENT_TYPE") != _env.end() && _env.find("CONTENT_LENGTH") == _env.end())
		Error::ThrowError("There is No Content Length");

	if (_env.find("CONTENT_TYPE") != _env.end() && _env.find("CONTENT_LENGTH") != _env.end())
	{
		if (Utility::strtosize_t(_env["CONTENT_LENGTH"], _content_len))
			;
		else
			Error::ThrowError("The Content Length Invalid");
		_Thereisbody = true;
		return;
	}
	_Thereisbody = false;
}

size_t Request::getContentLen() 
{
	return _content_len;
}

bool Request::ParseHeader()
{
	string line = "";
	string key;
	string value;

	if (_Parspos == eParsStart && getFullLine(line))
		parsHttpStandard(line);
	while (_Parspos == eParsHttpStand && getFullLine(line) && line != "\r\n")
	{
		if (line.find(':') != string::npos)
		{
			key = line.substr(0, line.find(':'));
			value = line.substr(line.find(':') + 1);
			Utility::trim(value, " \n\r");
		}
		else
			Error::ThrowError("Bad Request Header");
		if (key == "Host")
			Validation::parseListen(value, _env["SERVER_PORT"], _env["SERVER_NAME"]);
		if (_reqDirectives.find(key) != _reqDirectives.end())
			key = _reqDirectives[key];
		if (_env.find(key) != _env.end())
			Error::ThrowError("A Request Directive is Duplicate");
		else
			_env[key] = value;
	}
	if (_Parspos == eParsHttpStand && _env["REQUEST_METHOD"] == "POST") {
		parsLenTypeCont();
	}
	if (_Parspos == eParsHttpStand && line == "\r\n")
	{
		_Parspos = eParsEnd;
		if (_env["REQUEST_METHOD"] == "POST") {
			_env["Body"] = "";
			return fillBody();
		}
		return (true);
	}
	return (false);
}

bool Request::isRequestHeaderComplete()
{
	return _Parspos == eParsEnd;
}

string &Request::GetRequest()
{
	return _requestbuff;
}

string &Request::getBody() 
{
	return _env["Body"];
}

bool Request::fillBody()
{
	if (_Parspos == eParsEnd && _env["REQUEST_METHOD"] == "POST" && _Thereisbody)
	{
		_env["Body"] += _requestbuff;
		if (_env["Body"].size() < _content_len)
			return (false);
		else if (_env["Body"].size() > _content_len || _content_len > _maxbodysize)
			Error::ThrowError("413");
	}
	return (true);
}

bool Request::ParseRequest(string request_buff)
{
	_requestbuff += request_buff;
	if (_Parspos != eParsEnd)
	{
		if (!ParseHeader())
			return (false);
	}
	else
	{
		if (!fillBody())
			return (false);
	}
	return true;
}

bool Request::isComplete(char *request)
{
	if (!ParseRequest(request))
		return (false);
	return true;
}


map<string, string> &Request::getrequestenv()
{
	return (_env);
}

const string &Request::getMethod() const
{
	static const string empty = "";

	map<string, string>::const_iterator it =
		_env.find("REQUEST_METHOD");

	return (it != _env.end()) ? it->second : empty;
}

const string &Request::getHost() const
{
	static const string empty = "";

	map<string, string>::const_iterator it =
		_env.find("HTTP_HOST");

	return (it != _env.end()) ? it->second : empty;
}

const string &Request::getServerName() const
{
	static const string empty = "";

	map<string, string>::const_iterator it =
		_env.find("SERVER_NAME");

	return (it != _env.end()) ? it->second : empty;
}

const string &Request::getport() const
{
	static const string empty = "";

	map<string, string>::const_iterator it =
		_env.find("SERVER_PORT");

	return (it != _env.end()) ? it->second : empty;
}

void Request::SetMaxBodySize(int size)
{
	_maxbodysize = size;
	
	string &s = _env["Body"];
	if (s.size() > _maxbodysize || _content_len > _maxbodysize) {
		Error::ThrowError("413");
	}
}
