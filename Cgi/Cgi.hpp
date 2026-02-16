/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oel-bann <oel-bann@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/14 01:37:26 by oel-bann          #+#    #+#             */
/*   Updated: 2026/02/16 04:01:37 by oel-bann         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "../Headers.hpp"
#include "../SocketIO/SocketIO.hpp"

class Cgi
{
    Request     &_req;
    SocketIO    &_sok;
    pid_t       _pid;
    bool        _status;
    int         _infd;
    int         _outfd;
    long        _time;
    char        **_exec;
    Cgi();
    void createChild();
public:
    Cgi(int (&fds)[2], Request &req, char** exec, SocketIO &s);
    void run_cgi(int infd, int outfd, Request &req, char** exec);
    bool isExeted();
    void Handle();
    void resetTime();
    long getTime();
    ~Cgi();
    
};
