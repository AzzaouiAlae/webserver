/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiRequest.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aazzaoui <aazzaoui@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 08:51:20 by oel-bann          #+#    #+#             */
/*   Updated: 2026/03/03 05:48:49 by aazzaoui         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Headers.hpp"

class CgiRequest : public ARequest
{
    enum eCgiParsPos { eCgiHeaders, eCgiHeadersEnd, eCgiParsEnd};
    static map<string, string>_cgiDirectives;
    eCgiParsPos _parsPos;
    string _statusCode;
    void initReqDirectives();
    bool ParseHeader();
    void parseStatus();
    void parseLocation();
    void parsLenTypeCont();
    void checkCgiMinimum();
	size_t _headerSize;

public:
	size_t getRequestLen();
    CgiRequest();
    ~CgiRequest();
    string &getStatusCode();
    bool isComplete(char *request, int size);
};
