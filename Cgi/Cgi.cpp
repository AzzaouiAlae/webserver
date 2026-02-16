/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oel-bann <oel-bann@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/14 01:39:07 by oel-bann          #+#    #+#             */
/*   Updated: 2026/02/16 04:19:07 by oel-bann         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cgi.hpp"

Cgi::Cgi(int (&fds)[2], Request &req, char** exec, SocketIO &sok)
{
   _infd = fds[1];
   _outfd = fds[0];
   _req = req;
   _exec = exec;
   _sok = sok;
   _status = false;
}

bool Cgi::isExeted()
{
    int status;
    if (waitpid(_pid, &status, WNOHANG) != _pid)
        return false;
    return true;
}


void Cgi::resetTime()
{
    _time = Utility::CurrentTime();
}

long Cgi::getTime()
{
    return (_time);
}

//SocketIO
    //Request
    //Path
    //-----------------------
    // status 1 fork(); 
    // env pipe child
    // body 1 checkbody  parent 
    //----------------------
    // else
    // 
void Cgi::createChild()
{
    _pid = fork();
    if (_pid == -1)
        Error::ThrowError("Fork Failed");
    else if (!_pid)
    {
        Environment::CreateEnv(_req.getrequestenv());
        dup2(0, _infd);
        close(_infd);
        dup2(1, _outfd);
        close(_outfd);
        execve(_exec[0], _exec, environ);
        exit(1);
    }
}

void Cgi::Handle()
{
    if (!_status)
        createChild();
    if (_req.getthereisbody())
    {
        string &body = _req.getBody();
        _sok.SendBuffToPipe((void *)body.c_str(), body.size());
        _sok.SendPipeToSock();
    }

}
