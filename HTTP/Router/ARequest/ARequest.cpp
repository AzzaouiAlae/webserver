/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ARequest.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aazzaoui <aazzaoui@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 06:22:13 by oel-bann          #+#    #+#             */
/*   Updated: 2026/03/03 04:30:50 by aazzaoui         ###   ########.fr       */
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
