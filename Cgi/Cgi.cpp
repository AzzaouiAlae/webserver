/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aazzaoui <aazzaoui@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/14 01:39:07 by oel-bann          #+#    #+#             */
/*   Updated: 2026/03/01 06:33:20 by aazzaoui         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cgi.hpp"
#include <string.h>


Cgi::Cgi(ClientRequest &req, const char* exec, SocketIO *sok) : _req(req), _sok(sok)
{
    _exec = exec;
    _TimeSeted = false;
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
		char *args[] = {(char *)_exec, NULL};
		
        Environment::CreateEnv(_req.getrequestenv());
        dup2(_sok->pipefd[0], 0);
        close(_sok->pipefd[0]);
        dup2(_sok->pipefd[1], 1);
        close(_sok->pipefd[1]);
        execve(_exec, args, environ);
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
            len = _sok->SendBuffToPipe((void *)body.c_str(), body.size());
            _status = eSENDBUFFTOPIPE;
            _reqlen += len;
        }
        else if (!_eventexec && _status == eSENDBUFFTOPIPE)
        {
            len = _sok->SendSocketToPipe();
            if (len == 0)
            {
                if (_reqlen != _req.getcontentlen())
                    Error::ThrowError("Bad Request");
                _status = eFINISHWRITING;
            }
            _status = eSENDSOCKETOPIPE;
            _reqlen += len;
        }
    }
}


void Cgi::readfromcgi()
{
    char buf[MAXHEADERSIZE];
    // char *copybuf;

    if (_status == eFINISHWRITING)
    {
        if (_status < ePARSEDCGIHEADER)
        {
            int len = 0;
            if (_sok->CanUsePipe0())
                len = read(_sok->pipefd[0], buf, MAXHEADERSIZE - 1);
            if (len <= 0)
                Error::ThrowError("504");
            buf[len + 1] = '\0';
            // copybuf = strdup(buf);
            if (_cgireq.isComplete(buf, len + 1))
                _status = ePARSEDCGIHEADER;
        }
    }
}

void Cgi::Handle()
{
    if (!_TimeSeted)
    {
        resetTime();
        _TimeSeted = true;
    }
    if (_time - Utility::CurrentTime() > TIMEOUT * 1000000)
        Error::ThrowError("504");
    if (_status == eSTART)
        createChild();
    if (_status < eFINISHWRITING)
        writetocgi();
    if (_status >= eFINISHWRITING)
        readfromcgi();
}

Cgi::~Cgi()
{
}

CgiRequest   &Cgi::getCgiReq()
{
    return (_cgireq);
}
