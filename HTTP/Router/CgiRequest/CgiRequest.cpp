/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiRequest.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aazzaoui <aazzaoui@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 08:51:18 by oel-bann          #+#    #+#             */
/*   Updated: 2026/03/07 04:20:42 by aazzaoui         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiRequest.hpp"
#include "AMethod.hpp"

map<string, string> CgiRequest::_cgiDirectives;

void CgiRequest::initReqDirectives()
{
    if (_cgiDirectives.size() != 0)
        return;
    _cgiDirectives["Status"]            = "Status";
    _cgiDirectives["Location"]          = "Location";
    _cgiDirectives["Content-Type"]      = "Content-Type";
    _cgiDirectives["Content-Length"]    = "Content-Length";
}

CgiRequest::CgiRequest()
{
    _parsPos = eCgiHeaders;
}
CgiRequest::~CgiRequest() {}

void CgiRequest::parseStatus()
{
    map<string, string> statusMap = AMethod::getStatusMap();
    if (_env.find("Status") != _env.end())
    {
        stringstream ss(_env["Status"]);
        string status;
        string optionalmsg;
        string extra;
    
        if (!(ss >> status) || ((ss >> optionalmsg) && (ss >> extra)))
            Error::ThrowError("502");
        if (statusMap.find(status) == statusMap.end())
            Error::ThrowError("502");
        _statusCode = status;
        if (status[0] == '4' || status[0] == '5') {
		}
        else if (status[0] != '2' && status[0] != '3')
            Error::ThrowError("502");
    }
    else
    {
        if (_env.find("Location") == _env.end())
            _statusCode = "200";
        else
            _statusCode = "302";
    }
}

void CgiRequest::parseLocation()
{
    if (_env.find("Location") != _env.end())
    {
        if (_env["Location"].empty())
            Error::ThrowError("502");
        if (!(_env["Location"].find("http://") == 0 || _env["Location"].find("https://") == 0 || _env["Location"].find("/") == 0))
            Error::ThrowError("502");
    }
}

void CgiRequest::parsLenTypeCont()
{
    if (_env.find("Content-Length") != _env.end())
        if (!Utility::strtosize_t(_env["Content-Length"], _content_len))
			Error::ThrowError("502");
    if (getthereisbody())
    {
        if (_env.find("Content-Type") == _env.end())
            Error::ThrowError("502");
        if (_content_len == 0 || _env.find("Content-Length") == _env.end())
			Error::ThrowError("502");
    }
}

void CgiRequest::checkCgiMinimum()
{
    if (_env.find("Location") != _env.end() || _env.find("Content-Type") != _env.end())
        return;
    Error::ThrowError("502");
}

bool CgiRequest::ParseHeader()
{
    string line = "";

    while (_parsPos == eCgiHeaders && getFullLine(line) && line != "\r\n")
		parseHeaderLine(line);
    if (_parsPos == eCgiHeaders && line == "\r\n" && _requestbuff.length() > 0) _Thereisbody = true;
    if (_parsPos == eCgiHeaders && line == "\r\n")
        _parsPos = eCgiHeadersEnd;
    if (eCgiHeadersEnd)
    {
        checkCgiMinimum();
        parsLenTypeCont();
        parseStatus();
        parseLocation();
        _parsPos = eCgiParsEnd;
        return (true);
    }
    return (false);
}

string &CgiRequest::getStatusCode()
{
    return (_statusCode);
}


bool CgiRequest::isComplete(char *request, int size)
{
	_requestbuff.append(request, size);
	if (_parsPos != eCgiParsEnd)
	{
		if (!ParseHeader())
			return (false);
	}
	return true;
}

bool CgiRequest::getFullLine(string &line)
{
	int index = 0;
	line.clear();

	while (_requestbuff[index] && _requestbuff[index] != '\n')
	{
		if (index > MAXHEADERSIZE)
			Error::ThrowError("502");
		line += _requestbuff[index];
		index++;
	}
	if (_requestbuff[index] == '\n')
	{
		_headersize += index;
        if (_headersize > MAXHEADERSIZE)
			Error::ThrowError("502");
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

void CgiRequest::parseHeaderLine(const string &line)
{
    size_t colonPos = line.find(':');
    if (colonPos == string::npos)
        Error::ThrowError("502");
    string key   = line.substr(0, colonPos);
    string value = line.substr(colonPos + 1);
    Utility::trim(value, " \n\r");

    if (_reqDirectives && _reqDirectives->find(key) != _reqDirectives->end())
        key = (*_reqDirectives)[key];

    if (_env.find(key) != _env.end())
    {
        if (key != "Cookie") {
			Error::ThrowError("502");
		}
    }
    _env[key] = value;
}