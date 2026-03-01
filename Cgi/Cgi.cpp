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
#include "CGIPipe.hpp"
#include <string.h>


Cgi::Cgi(ClientRequest &req, const char* exec, SocketIO *sok) : _req(req), _sok(sok)
{
    pipe(_pipefd);
    _exec = exec;
    _TimeSeted = false;
    _reqlen  = 0;
    Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();

	_in = new CGIPipe(_pipefd[0], this);
	MulObj->AddAsEpollIn(_in);

	_out = new CGIPipe(_pipefd[1], this);
	MulObj->AddAsEpollOut(_out);
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
        close(_pipefd[0]);
        close(_sok->pipefd[0]);
        dup2(_pipefd[1], 1);
        close(_pipefd[1]);
        close(_sok->pipefd[1]);
        execve(_exec, args, environ);
        exit(1);
    }
    _status = eFORK;
}

void Cgi::writetocgi()
{
    int len = 0;
    size_t size = _req.getcontentlen() - _reqlen;

    if (_status < eFINISHWRITING && _req.getthereisbody())
    {
        if (_status == eFORK) 
        {
            string &body = _req.getBody();
            if (size > body.size() - _reqlen)
                len = _sok->SendBuffToPipe((char *)body.c_str() + _reqlen, body.size() - _reqlen);
            else
                len = _sok->SendBuffToPipe((char *)body.c_str() + _reqlen, size);
            if (_reqlen == body.size())
                _status = eSENDBUFFTOPIPE;
            _reqlen += len;
        }
        else if (_status == eSENDBUFFTOPIPE)
        {
            
            len = _sok->SendSocketToPipe(size);
            if (len == 0)
            {
                if (_sok->errorNumber == eReadError)
                    Error::ThrowError("502");
            }
            _status = eSENDSOCKETOPIPE;
            _reqlen += len;
        }
    }
    if (_reqlen >= _req.getcontentlen())
        _status = eFINISHWRITING;
}


void Cgi::readfromcgi()
{
    char buf[MAXHEADERSIZE];

    if (_status == eFINISHWRITING)
    {
        if (_status < ePARSEDCGIHEADER)
        {
            int len = 0;
            if (_sok->CanUsePipe0())
                return;
            len = read(_sok->pipefd[0], buf, MAXHEADERSIZE);
            if (len <= 0)
                Error::ThrowError("504");
            _copybuf.append(buf, len);
            if (_cgireq.isComplete(buf, len))
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

void Cgi::SetStateByFd(int fd)
{
	if (fd == _pipefd[0])
		_statusfd |= ePipe0;
	else if (fd == _pipefd[1])
		_statusfd |= ePipe1;
}

Cgi::~Cgi()
{
    Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();
    if (_in)
	{
		DDEBUG("HTTPContext") << "  -> Deleting in-pipe fd=" << _in->GetFd();
		MulObj->DeleteFromEpoll(_in);
		delete _in;
	}
	if (_out)
	{
		DDEBUG("HTTPContext") << "  -> Deleting out-pipe fd=" << _out->GetFd();
		MulObj->DeleteFromEpoll(_out);
		delete _out;
	}
}

CgiRequest   &Cgi::getCgiReq()
{
    return (_cgireq);
}
