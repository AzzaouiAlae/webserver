/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiRequest.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aazzaoui <aazzaoui@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 08:51:18 by oel-bann          #+#    #+#             */
/*   Updated: 2026/03/03 05:49:13 by aazzaoui         ###   ########.fr       */
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
	_headerSize = 0;
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
        if (status[0] == '4' || status[0] == '5')
            Error::ThrowError(status);
        if (status[0] != '2' || status[0] != '3')
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
    if (_env["Location"].empty())
        Error::ThrowError("502");
    if (!(_env["Location"].find("http://") == 0 || _env["Location"].find("https://") == 0 || _env["Location"].find("/") == 0))
        Error::ThrowError("502");
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

size_t CgiRequest::getRequestLen()
{
	return _headerSize;
}

bool CgiRequest::ParseHeader()
{
    string line = "";
	bool isFullLine;

    while (_parsPos == eCgiHeaders)
	{
		isFullLine = getFullLine("cgi", line);
		
		if (isFullLine == false)
			break;
		_headerSize += line.length();
		if (line == "\r\n")
			break;
		parseHeaderLine("cgi", line);
	}
    if (_parsPos == eCgiHeaders && line == "\r\n" && _requestbuff.length() > 0) _Thereisbody = true;
    if (_parsPos == eCgiHeaders && line == "\r\n")
        _parsPos = eCgiHeadersEnd;
    if (eCgiHeadersEnd)
    {
        checkCgiMinimum();
        parsLenTypeCont();
        parseStatus();
        // parseLocation();
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