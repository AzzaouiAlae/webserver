/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ARequest.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oel-bann <oel-bann@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 06:22:13 by oel-bann          #+#    #+#             */
/*   Updated: 2026/02/19 08:19:46 by oel-bann         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ARequest.hpp"

ARequest::ARequest()
{
    _reqDirectives = NULL;
    _requestbuff = "";
    _content_len = 0;
    _Thereisbody = false;
	_headersize = 0;
}

ARequest::~ARequest()
{
}

bool ARequest::getFullLine(string &line)
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
		_headersize += index;
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

void ARequest::parseHeaderLine(const string &line)
{
    size_t colonPos = line.find(':');
    if (colonPos == string::npos)
        Error::ThrowError("Bad Request Header");

    string key   = line.substr(0, colonPos);
    string value = line.substr(colonPos + 1);
    Utility::trim(value, " \n\r");

    if (_reqDirectives && _reqDirectives->find(key) != _reqDirectives->end())
        key = (*_reqDirectives)[key];

    if (_env.find(key) != _env.end())
        Error::ThrowError("Duplicate header: " + key);
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

Session* ARequest::getSession() {
    if (_currentSession == NULL) {
        initSession();
    }
    return _currentSession;
}
