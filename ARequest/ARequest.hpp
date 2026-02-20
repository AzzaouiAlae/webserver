/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ARequest.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oel-bann <oel-bann@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 06:22:09 by oel-bann          #+#    #+#             */
/*   Updated: 2026/02/19 11:29:51 by oel-bann         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "../Headers.hpp"

#define MAXHEADERSIZE 8192

class ARequest
{
protected:
    map<string, string> *_reqDirectives;
    map<string, string> _env;
    map<string, string> _cookies;
    Session* _currentSession;
    string _requestbuff;
	size_t _content_len;
    bool _Thereisbody;
    int  _headersize;

    bool getFullLine(string &line);
    void parseHeaderLine(const string &line);
    virtual bool ParseHeader() = 0;
    virtual void initReqDirectives() = 0;
    virtual void parsLenTypeCont() = 0;
    void parseCookies();
public:
    ARequest();
    virtual ~ARequest();
    virtual bool isComplete(char *request, int size) = 0;
    
    map<string, string> &getrequestenv();
    bool 		        getthereisbody();
    size_t		        getcontentlen();
    string              &getBody();
    private:
    
    void initSession();
    Session* getSession();
    string getCookie(const string &name);

};
