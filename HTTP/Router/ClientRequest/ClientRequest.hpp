/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientRequest.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aazzaoui <aazzaoui@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 20:05:48 by oel-bann          #+#    #+#             */
/*   Updated: 2026/03/03 03:55:22 by aazzaoui         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Headers.hpp"


class ClientRequest: public ARequest {

	enum eHeaderParsPos {eParsStart, eParsHttpStand, eParsEnd};
	static map<string, string> _clientreqDirectives;
	eHeaderParsPos _Parspos;
	size_t _maxbodysize;
	void initReqDirectives();
	bool ParseHeader();
	void parsHttpStandard(string httpStandard);
	bool parsPath(string path);
	void parseHost();
	void parsLenTypeCont();
	bool getFullLine(string &line);
	void parseHeaderLine(const string &line);
public:
	ClientRequest(size_t maxbodysze);
	ClientRequest();
	~ClientRequest();
	bool isComplete(char *request, int size);
	const string &getMethod() const;
	const string &getport() const;
	const string &getHost() const;
	const string &getServerName() const;
	void 		 setUrlPart(string scriptpath, string pathinfo);
	string		 &getPath();
	void SetMaxBodySize(int size);
	bool isRequestHeaderComplete();
};

