/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oel-bann <oel-bann@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 20:05:48 by oel-bann          #+#    #+#             */
/*   Updated: 2026/02/07 23:50:14 by oel-bann         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#define MAXHEADERSIZE  8192

#include "../Headers.hpp"

class Request
{
	map<string, string> _env;
	static map<string, string> _reqDirectives;
	map<string, void (Request::*)(string)> _Handlers;
	bool	_headerparsed;
	string	_method;
	string	_host;
	string	_path;
	string	_port;
	string	_query_s;
	string	_content_len;
	string	_content_type;
	string	_body;
	string	_request;
	int 	_pos;
	void initReqDirectives();
	bool ParseHeader();
	void parsHttpStandard(string httpStandard);
	bool parsPath(string path);
	void parsHost(string Host);
	void handleGet(string);
    void handlePost(string);
    void handleDelete(string);
public:
	bool getFullLine (string &line);
	string	_headerbuff;
    Request();
	bool ParseRequest(string request_buff);
	bool isComplete(char *request);
};