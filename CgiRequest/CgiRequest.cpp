/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiRequest.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oel-bann <oel-bann@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 08:51:18 by oel-bann          #+#    #+#             */
/*   Updated: 2026/02/20 03:28:28 by oel-bann         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiRequest.hpp"

void CgiRequest::initDirectives()
{
    if (_cgiDirectives.size() != 0)
        return;
    _cgiDirectives["Status"]            = "CGI_STATUS";
    _cgiDirectives["Location"]          = "CGI_LOCATION";
    _cgiDirectives["Content-Type"]      = "CONTENT_TYPE";
    _cgiDirectives["Content-Length"]    = "CONTENT_LENGTH";
    _cgiDirectives["Set-Cookie"]        = "CGI_SET_COOKIE";
    _cgiDirectives["Expires"]           = "EXPIRES";
    _cgiDirectives["Last-Modified"]     = "LAST_MODIFIED";
}

CgiRequest::CgiRequest()
{
    _parsPos = eCgiHeaders;
}
CgiRequest::~CgiRequest() {}

bool CgiRequest::ParseHeader()
{
    string line = "";

    while (_parsPos == eCgiHeaders && getFullLine(line) && line != "\r\n")
		parseHeaderLine(line);
    if (_parsPos == eCgiHeaders && line == "\r\n" && _requestbuff.length() > 0) _Thereisbody = true;
}