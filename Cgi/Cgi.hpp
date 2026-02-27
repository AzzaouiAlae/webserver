/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oel-bann <oel-bann@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/14 01:37:26 by oel-bann          #+#    #+#             */
/*   Updated: 2026/02/19 02:54:38 by oel-bann         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "../SocketIO/SocketIO.hpp"
#include "../Headers.hpp"

enum estatus
{
    eSTART,
    eFORK,
    eSENDBUFFTOPIPE,
    eSENDSOCKETOPIPE,
    eFINISHWRITING,
    ePARSEDCGIHEADER,

    eSENDPIPETOSOCKET,
    eCOMPLETE
};

class Cgi
{
    ClientRequest     &_req;
    CgiRequest        _cgireq;
    SocketIO    *_sok;
    pid_t       _pid;
    estatus     _status;
    bool        _parsheader;
    long        _time;
    bool        _TimeSeted;
    bool        _eventexec;
    const char        *_exec;
    size_t      _reqlen;
    void createChild();
    void writetocgi();
    void readfromcgi();
    Cgi();
public:
    Cgi(ClientRequest &req, const char* exec, SocketIO *sok);
    bool isExeted();
    void Handle();
    void resetTime();
    long getTime();
    CgiRequest   &getCgiReq();
    ~Cgi();
    
};
