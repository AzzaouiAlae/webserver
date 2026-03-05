/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aazzaoui <aazzaoui@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/14 01:37:26 by oel-bann          #+#    #+#             */
/*   Updated: 2026/03/05 01:51:00 by aazzaoui         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "SocketIO.hpp"
#include "Headers.hpp"

#define CGILog(lvl) lvl("Cgi") << "[CGI PID: " << _pid << "] "

enum estatus
{
    eFork,
	eSendBuffToPipe,
	eSendSockToPipe,
	eReadCgiResponse,
	eCreateResponseHeader,
	eWriteBuffToClient,
	eWritePipeToClient,
	eComplete
};

class Cgi
{
    ClientRequest     &_req;
    CgiRequest        _cgireq;
    SocketIO    *_sok;
    pid_t       _pid;
    estatus     _status;
    int         _statusfd;
    string      _copybuf;
    AFd         *_out;
	AFd         *_in;
    int        _pipefd[2];
    bool        _parsheader;
	string		_responseHeaderStr;
	char		*_buf;
    char        *const*_exec;
    size_t      _reqlen;
	size_t		_responselen;
    void createChild();
    void writetocgi();
    void readfromcgi();
	void _activeCgiPipe();
    Cgi();
	size_t _shouldSend;
public:
    Cgi(ClientRequest &req, char* const* exec, SocketIO *sok);
    bool isChildError();
    void Handle();
    CgiRequest   &getCgiReq();
    void SetStateByFd(int fd);
    string &getStatusCode();
    bool CanUsePipe0();
    bool CanUsePipe1();
	void createCgiResponse();
	void writeToClientSoket();
	bool isComplete();
    ~Cgi();
};
