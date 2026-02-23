/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ARequest.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aazzaoui <aazzaoui@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 06:22:09 by oel-bann          #+#    #+#             */
/*   Updated: 2026/02/20 05:06:20 by aazzaoui         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "iostream"
#include <map>
#include <vector>
using namespace std;
#define MAXHEADERSIZE 8192

class ARequest
{
protected:
    map<string, string> *_reqDirectives;
    map<string, string> _env;
    string _requestbuff;
	size_t _content_len;
    bool _Thereisbody;
    int  _headersize;

    bool getFullLine(string &line);
    void parseHeaderLine(const string &line);
    virtual bool ParseHeader() = 0;
    virtual void initReqDirectives() = 0;
public:
    ARequest();
    virtual ~ARequest();
    virtual bool isComplete(char *request, int size) = 0;
    
    map<string, string> &getrequestenv();
    bool 		        getthereisbody();
    size_t		        getcontentlen();
    string              &getBody();
};

#include "../Headers.hpp"
