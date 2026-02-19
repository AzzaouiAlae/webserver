/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oel-bann <oel-bann@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/14 01:37:26 by oel-bann          #+#    #+#             */
/*   Updated: 2026/02/16 12:09:38 by oel-bann         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "../Headers.hpp"
#include "../SocketIO/SocketIO.hpp"

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
    Request     &_req;
    SocketIO    &_sok;
    pid_t       _pid;
    bool        _status;
    bool        _parsheader;
    long        _time;
    bool        _eventexec;
    char        **_exec;
    size_t      _reqlen;
    Cgi();
    void createChild();
    void writetocgi();
    void readfromcgi();
public:
    Cgi(Request &req, char** exec, SocketIO &sok);
    void run_cgi(int infd, int outfd, Request &req, char** exec);
    bool isExeted();
    void Handle();
    void resetTime();
    long getTime();
    ~Cgi();
    
};
