/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ARequest.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aazzaoui <aazzaoui@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 06:22:13 by oel-bann          #+#    #+#             */
/*   Updated: 2026/03/06 14:48:41 by aazzaoui         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ARequest.hpp"


ARequest::ARequest()
{
    _reqDirectives = NULL;
    _requestbuff = "";
    _content_len = 0;
    _firstline = true;
    _Thereisbody = false;
	_headersize = 0;
    _currentSession = NULL;
    _isChunked = false;
    _chunkedComplete = false;
    _chunkState = eChunkSize;
    _currentChunkSize = 0;
    _currentChunkRead = 0;
    _isMultipart = false;
}

ARequest::~ARequest()
{}


bool ARequest::getthereisbody()
{
	return (_Thereisbody);
}

string &ARequest::getBody()
{
	return (_requestbuff);
}

size_t		 ARequest::getcontentlen()
{
	return (_content_len);
}

map<string, string> &ARequest::getrequestenv()
{
	return (_env);
}

void ARequest::parseCookies() {
    string cookieHeader = _env["HTTP_COOKIE"];
    if (cookieHeader.empty()) return;
    
    stringstream ss(cookieHeader);
    string cookie;
    while (getline(ss, cookie, ';')) {
        size_t start = cookie.find_first_not_of(" \t");
        if (start == string::npos) continue;
        cookie = cookie.substr(start);
        
        size_t eqPos = cookie.find('=');
        if (eqPos != string::npos) {
            string name = cookie.substr(0, eqPos);
            string value = cookie.substr(eqPos + 1);
            _cookies[name] = value;
        }
    }
}

string ARequest::getCookie(const string &name) {
    if ( _cookies.find(name) != _cookies.end())
        return _cookies[name];
    return "";
}

void ARequest::initSession() {
    parseCookies();
    
    SessionManager* sm = SessionManager::getInstance();
    string sessionId = getCookie("SESSIONID");
    
    if (!sessionId.empty()) {
        _currentSession = sm->getSession(sessionId);
	
    }
    
    if (_currentSession == NULL) {
        _currentSession = sm->createSession();
		_currentSession->data["SESSIONID"] = _currentSession->id;
    }
    
    map<string, string>::iterator it = _cookies.begin();
    for (; it != _cookies.end(); ++it) {
        if (it->first != "SESSIONID") {
            _currentSession->data[it->first] = it->second;
        }
    }
    _env["SESSIONID"] = _currentSession->id;
}
Session* ARequest::getSession() {
    if (_currentSession == NULL) {
        initSession();
    }
    return _currentSession;
}

bool ARequest::isChunkedTransferEncoding()
{
    if (_env.find("HTTP_TRANSFER_ENCODING") != _env.end())
        return _env["HTTP_TRANSFER_ENCODING"] == "chunked";
    if (_env.find("Transfer-Encoding") != _env.end())
        return _env["Transfer-Encoding"] == "chunked";
    return false;
}

bool ARequest::isMultipartFormData()
{
    string ct;
    if (_env.find("CONTENT_TYPE") != _env.end())
        ct = _env["CONTENT_TYPE"];
    else if (_env.find("Content-Type") != _env.end())
        ct = _env["Content-Type"];
    if (ct.empty())
        return false;
    return ct.find("multipart/form-data") != string::npos;
}

string ARequest::getMultipartBoundary()
{
    string ct;
    if (_env.find("CONTENT_TYPE") != _env.end())
        ct = _env["CONTENT_TYPE"];
    else if (_env.find("Content-Type") != _env.end())
        ct = _env["Content-Type"];
    if (ct.empty())
        return "";
    size_t pos = ct.find("boundary=");
    if (pos == string::npos)
        return "";
    string boundary = ct.substr(pos + 9);
    if (!boundary.empty() && boundary[0] == '"')
    {
        size_t end = boundary.find('"', 1);
        if (end != string::npos)
            boundary = boundary.substr(1, end - 1);
    }
    return boundary;
}

bool ARequest::processChunkedData()
{
    while (_chunkState != eChunkDone)
    {
        if (_chunkState == eChunkSize)
        {
            size_t pos = _requestbuff.find("\r\n");
            if (pos == string::npos)
                return false;
            string sizeLine = _requestbuff.substr(0, pos);
            _requestbuff.erase(0, pos + 2);
            size_t semiPos = sizeLine.find(';');
            if (semiPos != string::npos)
                sizeLine = sizeLine.substr(0, semiPos);
            char *endptr;
            _currentChunkSize = strtoul(sizeLine.c_str(), &endptr, 16);
            if (*endptr != '\0' && *endptr != ' ')
                Error::ThrowError("400");
            _currentChunkRead = 0;
            if (_currentChunkSize == 0)
                _chunkState = eChunkTrailer;
            else
                _chunkState = eChunkData;
        }
        else if (_chunkState == eChunkData)
        {
            size_t remaining = _currentChunkSize - _currentChunkRead;
            size_t available = _requestbuff.length();
            size_t toRead = min(remaining, available);
            if (toRead == 0) {
                if (remaining == 0 && _requestbuff.length() >= 2) {
                    if (_requestbuff[0] != '\r' || _requestbuff[1] != '\n')
                        Error::ThrowError("400");
                    _requestbuff.erase(0, 2);
                    _chunkState = eChunkSize;
                    continue;
                }
                else
                    return false;
            }
            _decodedBody.append(_requestbuff, 0, toRead);
            _requestbuff.erase(0, toRead);
            _currentChunkRead += toRead;
            if (_currentChunkRead >= _currentChunkSize)
            {
                if (_requestbuff.length() < 2)
                    return false;
                if (_requestbuff[0] != '\r' || _requestbuff[1] != '\n')
                    Error::ThrowError("400");
                _requestbuff.erase(0, 2);
                _chunkState = eChunkSize;
            }
        }
        else if (_chunkState == eChunkTrailer)
        {
            size_t pos = _requestbuff.find("\r\n");
            if (pos == string::npos)
                return false;
            if (pos == 0)
            {
                _requestbuff.erase(0, 2);
                _chunkState = eChunkDone;
                _chunkedComplete = true;
                _content_len = _decodedBody.length();
            }
            else
                _requestbuff.erase(0, pos + 2);
        }
    }
    return true;
}

bool ARequest::isChunkedComplete() const
{
    return _chunkedComplete;
}

string &ARequest::getDecodedBody()
{
    return _decodedBody;
}
