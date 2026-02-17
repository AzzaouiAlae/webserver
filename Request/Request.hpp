/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aazzaoui <aazzaoui@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 20:05:48 by oel-bann          #+#    #+#             */
/*   Updated: 2026/02/17 21:36:11 by aazzaoui         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#define MAXHEADERSIZE 8192

#include "../Headers.hpp"

enum eHeaderParsPos
{
	eParsStart,
	eParsHttpStand,
	eParsEnd,
};

class Request
{
	map<string, string> _env;
	static map<string, string> _reqDirectives;
	map<string, void (Request::*)(string)> _Handlers;
	string _requestbuff;
	size_t _content_len;
	size_t _maxbodysize;
	string _request;
	int _Parspos;
	bool _Thereisbody;
	void initReqDirectives();
	bool ParseRequest(string request_buff);
	bool ParseHeader();
	void parsHttpStandard(string httpStandard);
	bool parsPath(string path);
	void parsLenTypeCont();
	bool fillBody();
	bool getFullLine(string &line);

public:
	Request(size_t maxbodysze);
	Request();
	bool isComplete(char *request);
	map<string, string> &getrequestenv();
	const string &getMethod() const;
	const string &getport() const;
	const string &getHost() const;
	const string &getServerName() const;
	size_t getContentLen() ;
	string &getBody();
	string &getPath();
	string &GetRequest();
	void SetMaxBodySize(int size);
	bool isRequestHeaderComplete();
};