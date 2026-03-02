/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiRequest.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oel-bann <oel-bann@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 08:51:20 by oel-bann          #+#    #+#             */
/*   Updated: 2026/02/19 11:49:44 by oel-bann         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../Headers.hpp"

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

public:
    CgiRequest();
    ~CgiRequest();
    string &getStatusCode();
    bool isComplete(char *request, int size);
};
