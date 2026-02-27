/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ARequest.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aazzaoui <aazzaoui@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 06:22:13 by oel-bann          #+#    #+#             */
/*   Updated: 2026/02/20 05:01:27 by aazzaoui         ###   ########.fr       */
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
}

ARequest::~ARequest()
{
}

bool ARequest::getFullLine(string reqtype, string &line)
{
	int index = 0;
	line.clear();

	while (_requestbuff[index] && _requestbuff[index] != '\n')
	{
		if (index > MAXHEADERSIZE)
        {
            if (reqtype == "cgi")
			    Error::ThrowError("502");
            if (_firstline)
                Error::ThrowError("414");
            Error::ThrowError("431");
        }
		line += _requestbuff[index];
		index++;
	}
    _firstline = false;
	if (_requestbuff[index] == '\n' && (index == 0 || (index > 0 && _requestbuff[index - 1] != '\r')))
    {
        if (reqtype == "cgi")
			Error::ThrowError("502");
        Error::ThrowError("400");
    }
	else if (_requestbuff[index] == '\n')
	{
		_headersize += index;
        if (_headersize > MAXHEADERSIZE)
        {
            if (reqtype == "cgi")
			    Error::ThrowError("502");
            Error::ThrowError("431");
        }
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

void ARequest::parseHeaderLine(string reqtype, const string &line)
{
    size_t colonPos = line.find(':');
    if (colonPos == string::npos)
    {
        if (reqtype == "cgi")
			Error::ThrowError("502");
        Error::ThrowError("400");
    }
    string key   = line.substr(0, colonPos);
    string value = line.substr(colonPos + 1);
    Utility::trim(value, " \n\r");

    if (_reqDirectives && _reqDirectives->find(key) != _reqDirectives->end())
        key = (*_reqDirectives)[key];

    if (_env.find(key) != _env.end())
    {
        if (key != "Cookie")
        {
            if (reqtype == "cgi")
			    Error::ThrowError("502");
            Error::ThrowError("400");
        }
    }
    _env[key] = value;
}


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
    return _cookies[name];
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
    }
    
    _env["SESSIONID"] = _currentSession->id;
    _env["HTTP_SESSIONID"] = _currentSession->id;
}

Session* ARequest::getSession() {
    if (_currentSession == NULL) {
        initSession();
    }
    return _currentSession;
}
