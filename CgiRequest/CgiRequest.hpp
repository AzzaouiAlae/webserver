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

class CgiRequest : ARequest
{
    enum eCgiParsPos { eCgiHeaders, eCgiBody, eCgiComplete };
    static map<string, string>_cgiDirectives;
    eCgiParsPos _parsPos;
    void initDirectives();
    bool ParseHeader();
    void parseStatus();
    void parseLocation();
    void parsLenTypeCont();

public:
    CgiRequest();
    ~CgiRequest();
};
