/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oel-bann <oel-bann@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/14 01:39:07 by oel-bann          #+#    #+#             */
/*   Updated: 2026/02/19 02:55:46 by oel-bann         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cgi.hpp"

Cgi::Cgi(Request &req, char** exec, SocketIO &sok) : _req(req), _sok(sok)
{
    _exec = exec;
    _reqlen  = 0;
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

void Cgi::createChild()
{
    _pid = fork();
    if (_pid == -1)
        Error::ThrowError("Fork Failed");
    else if (!_pid)
    {
        Environment::CreateEnv(_req.getrequestenv());
        dup2(_sok.pipefd[0], 0);
        close(_sok.pipefd[0]);
        dup2(_sok.pipefd[1], 1);
        close(_sok.pipefd[1]);
        execve(_exec[0], _exec, environ);
        exit(1);
    }
    _status = eFORK;
}

void Cgi::writetocgi()
{
    int len = 0;

    if (_status < eFINISHWRITING && _req.getthereisbody())
    {
        if (!_eventexec && _status == eFORK) 
        {
            string &body = _req.getBody();
            len = _sok.SendBuffToPipe((void *)body.c_str(), body.size());
            _status = eSENDBUFFTOPIPE;
            _reqlen += len;
        }
        else if (!_eventexec && _status == eSENDBUFFTOPIPE)
        {
            len = _sok.SendSocketToPipe();
            if (len == 0)
            {
                if (_reqlen != _req.getcontentlen())
                    Error::ThrowError("Bad Request");
                _status == eFINISHWRITING;
            }
            _status = eSENDSOCKETOPIPE;
            _reqlen += len;
        }
    }
}

void Cgi::readfromcgi()
{
    if (_status < ePARSEDCGIHEADER)
    {
        while (_status != ePARSEDCGIHEADER)
        {
            int len = read(_sok.pipefd[0], )
        }
    }
    
    if (_finishwriting && !_finishreading)
    {
        if (_sok.SendPipeToSock() == 0)
            _complete = true;
    }
}

void Cgi::Handle()
{
    _eventexec = false;
    if (_status == eSTART)
        createChild();
    if (_status < eFINISHWRITING)
        writetocgi();
    
}
