/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aazzaoui <aazzaoui@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/14 01:37:26 by oel-bann          #+#    #+#             */
/*   Updated: 2026/03/05 05:05:19 by aazzaoui         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "SocketIO.hpp"
#include "Headers.hpp"

#define CGILog(lvl) lvl("Cgi") << "[CGI PID: " << _pid << "] "

enum estatus
{
	eBufferChunkedBody,
    eFork,
	eSendBuffToPipe,
	eSendSockToPipe,
	eReadCgiResponse,
	eBufferCgiResponse,
	eCreateResponseHeader,
	eWriteBuffToClient,
	eWritePipeToClient,
	eComplete
};

class Cgi
{
    Routing *_router;
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
    size_t      _reqlen;
	size_t		_responselen;
    Multiplexer *_multiplexer;
    int _stdin;
    void createChild();
    void readfromcgi();
	void _activeCgiPipe();
	string resolveExcPath(const std::string &excName);
    Cgi();
	size_t _shouldSend;
    int _childErrorStatus;
    void ErrorHandler();
    void sendBuffToPipe();
    void sendSockToPipe();
    void checkWriteToCgiComplete();
    void writeBuffToClient();
    void writePipeToClient();
    void bufferCgiResponse();
    void finalizeAndSendBufferedResponse();
    void changeDir();
	void queueChunk(const char *data, size_t len);
	bool flushChunkedOut();
	void queueFinalChunk();

    // Chunked body buffering to temp file
    bool _isChunkedRequest;
    int _tmpFd;
    string _tmpPath;
    void bufferChunkedBodyToFile();
    void finalizeChunkedBodyFile();

    // Chunked response when CGI doesn't provide Content-Length
    bool _isChunkedResponse;
    bool _chunkedFinalQueued;
    bool _chunkedFinalSent;
    string _chunkedOut;
    size_t _chunkedOutSent;
    size_t _cgiBodyOffset;
    void _clearChunkedOut();

    // Buffered response when chunked_send is disabled
    bool _bufferedResponse;
    int _responseTmpFd;
    string _responseTmpPath;
    size_t _responseBodyLen;
    size_t _responseSentFromFile;
    bool _initialBodyBuffered;
    bool _endSend;

public:
    Cgi(Routing *router, SocketIO *sok);
    int isChildError();
    void Handle();
    CgiRequest   &getCgiReq();
    void SetStateByFd(int fd);
    string &getStatusCode();
    bool CanUsePipe0();
    bool CanUsePipe1();
	void createCgiResponse();
	void checkWriteToClientSoket();
	bool isComplete();
    ~Cgi();
};
