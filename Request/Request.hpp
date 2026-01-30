/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oel-bann <oel-bann@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 20:05:48 by oel-bann          #+#    #+#             */
/*   Updated: 2026/01/30 19:11:35 by oel-bann         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "iostream"
using namespace std;

#include "../Headers.hpp"

class Request
{
	map<string, string> _env;
	static map<string, string> _reqDirectives;
	map<string, void (Request::*)(string)> _Handlers;
	bool	_fist_buff;
	string	_method;
	string	_host;
	string	_path;
	string	_port;
	string	_query_s;
	string	_content_len;
	string	_content_type;
	string	_body;
	string	_request;
	void ParseHeader(istringstream iss);
	void parsHttpStandard(string httpStandard);
	void handleGet(std::string);
    void handlePost(std::string);
    void handleDelete(std::string);
public:
    Request();
	void ParseRequest(string request_buff);
	bool isComplete(char *request);
};