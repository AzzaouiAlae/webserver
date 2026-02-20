/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientRequest.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aazzaoui <aazzaoui@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 20:05:48 by oel-bann          #+#    #+#             */
/*   Updated: 2026/02/20 05:04:59 by aazzaoui         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../Headers.hpp"


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

