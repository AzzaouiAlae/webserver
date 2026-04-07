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
#include "NetIO.hpp"
#include "Headers.hpp"
#include "ReadPipeStrategy.hpp"

#define CGILog(lvl) lvl("Cgi") << "[CGI PID: " << _pid << "] "

class Cgi
{
    // Core members
    Routing *_router;
    ClientRequest &_req;
    CgiRequest _cgireq;
    int _status;
    AFd *_in;
    int _pipefd[2];
    int _pipe2fd[2];
    Multiplexer *_multiplexer;
    string _errorCode;
    bool _activeSocketIn;
    bool _activeSocketOut;
    bool _activePipeOut;
    void _activeFds();

    //create child related members
    int _stdin;
    pid_t _pid;
    void _createChild();
    void _prepareChild(string &exec);
    string _resolveExcPath(const std::string &excName);
    void _execChild(string &exec);
    void _changeDir();
    void _closeFds(int pipe1[2], int pipe2[2]);
    

    //create child related members
    AStrategy *_readStrategy;
    char *_buffer;
    int _availableClientBufferLen;
    ClientSocket *_sok;
    size_t _totalReadFromClient;
    void _readFromClient();

    // Write strategy for sending data to CGI process
    AStrategy *_writePipeStrategy;
    AFd *_out;
    size_t _totalWritenToPipe;
    void _sendBuffToPipe();
    
    // Read strategy for reading data from CGI process
    AStrategy *_readPipeStrategy;
    char *_readBuffer;
    size_t _availableCgiBufferLen;
    bool _headerParsed;
    size_t _totalReadFromPipe;
    void _readFromCGI();

    //write strategy for sending data to client
    AStrategy *_writeStrategy;
    size_t _totalSendToClient;
    bool _pipeEof;
    void _writeToClient();

    // Error handling
    int _childErrorStatus;
    void _errorHandler();
    void _handleStrategyStatus(AStrategy *strategy);
    int _isChildError();
public:
    enum Status
    {
        eFork,
        eHandleClient,
        eWriteToClientComplete,
        eComplete,
        eError,
    };

    Cgi(Routing *router, ClientSocket *sok);
    ~Cgi();
    void Handle();
    void SetStateByFd(int fd);
    void PipeClosed(int fd);
    bool isComplete();
    int getStatus() const;
    string getErrorCode() const;
};
