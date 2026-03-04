/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aazzaoui <aazzaoui@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/14 01:39:07 by oel-bann          #+#    #+#             */
/*   Updated: 2026/03/04 01:00:42 by aazzaoui         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cgi.hpp"
#include "CGIPipe.hpp"
#include "AMethod.hpp"
#include <string.h>

Cgi::Cgi(ClientRequest &req,  char *const *exec, SocketIO *sok) : _req(req), _sok(sok)
{
	pipe(_pipefd);
	_statusfd = 0;
	_status = eFork;
	_exec = exec;
	_TimeSeted = false;
	_reqlen = 0;
	_responseHeaderStr.clear();
	_responselen = 0;
	_buf = new char[MAXHEADERSIZE];
	_shouldSend=0;
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();

	_in = new CGIPipe(_pipefd[0], this);
	MulObj->AddAsEpollIn(_in);

	_out = new CGIPipe(_pipefd[1], this);
	MulObj->AddAsEpollOut(_out);
}

void Cgi::_activeCgiPipe()
{
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();

	MulObj->ChangeToEpollIn(_in);
	MulObj->ChangeToEpollOut(_out);
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

void Cgi::createChild()
{
	_pid = fork();
	if (_pid == -1)
		Error::ThrowError("500");
	else if (_pid == 0)
	{
		Environment::CreateEnv(_req.getrequestenv());
		dup2(_sok->pipefd[0], 0);
		close(_pipefd[0]);
		close(_sok->pipefd[0]);
		dup2(_pipefd[1], 1);
		close(_pipefd[1]);
		close(_sok->pipefd[1]);
		execve(*_exec, _exec, environ);
		exit(1);
	}
	_status = eSendBuffToPipe;
}

void Cgi::writetocgi()
{
	int len = 0;
	size_t size = _req.getcontentlen() - _reqlen;

	if (_req.getthereisbody())
	{
		if (_status == eSendBuffToPipe)
		{
			string &body = _req.getBody();
			void *b =  (char *)body.c_str() + _reqlen;
			if (size > body.size() - _reqlen)
				len = _sok->SendBuffToPipe(b, body.size() - _reqlen, false);
			else
				len = _sok->SendBuffToPipe(b, size, false);

			if (_sok->errorNumber == ePipe1Error)
				Error::ThrowError("502");
			_reqlen += len;
			if (_reqlen == body.size())
				_status = eSendSockToPipe;
		}
		else if (_status == eSendSockToPipe)
		{
			len = _sok->SendSocketToPipe(size, false);
			if (_sok->errorNumber == eReadError)
				Error::ThrowError("502");
			_reqlen += len;
		}
	}

	if (_reqlen >= _req.getcontentlen() || _req.getthereisbody() == false) {
		_status = eReadCgiResponse;
		Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();
		MulObj->ChangeToEpollOut(_sok);
	}
}

void Cgi::readfromcgi()
{
	int len = 0;
	if (!CanUsePipe0())
		return;
	len = read(_pipefd[0], _buf, MAXHEADERSIZE);
	if (len <= 0)
		Error::ThrowError("504");
	if (_cgireq.isComplete(_buf, len))
		_status = eCreateResponseHeader;
	
}

void Cgi::createCgiResponse()
{
	string body(_buf); 
	_responseHeaderStr = "HTTP/1.1 " + getStatusCode() + " " + AMethod::getStatusMap()[getStatusCode()] + "\r\n";
	map<string, string> &env = _cgireq.getrequestenv();
	for(map<string,string>::iterator it = env.begin(); it != env.end(); it++)
		_responseHeaderStr += it->first + ": " + it->second + "\r\n";
	_responseHeaderStr += "\r\n";
	_responseHeaderStr += body;
	_shouldSend = _responseHeaderStr.length() + body.length();
	_status = eWriteBuffToClient;
}

void Cgi::writeToClientSoket()
{
	string body(_buf);
	if (_status == eWriteBuffToClient)
	{
		int len = 0;
		void *buff = (void *)(_responseHeaderStr.c_str() + _responselen);
		len = _sok->Send(buff, _shouldSend - _responselen);
		if (_sok->errorNumber == eWriteError)
			Error::ThrowError("502");
		_responselen += len;
		if (_responselen == _shouldSend)
			_status = eWritePipeToClient;
	}
	else if (_status == eWritePipeToClient)
	{
		int len = 0;
		if (!CanUsePipe0())
			return;
		len = _sok->SendPipeToSock(_pipefd[0], (_cgireq.getcontentlen() - body.length()) - _responselen);
			if (_sok->errorNumber == eWriteError)
				Error::ThrowError("502");
			_responselen += len;
		}
	if ((_cgireq.getcontentlen() - body.length()) <= _responselen)
		_status = eComplete;
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
	if (_status == eFork)
		createChild();
	// if (isExeted() && _status < eCreateResponseHeader)
	// 	Error::ThrowError("500");
	if (_status < eReadCgiResponse)
		writetocgi();
	else if (_status == eReadCgiResponse)
		readfromcgi();
	if (_status == eCreateResponseHeader)
		createCgiResponse();
	else if (_status > eCreateResponseHeader)
		writeToClientSoket();
	if (_status != eComplete)
		_activeCgiPipe();
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
		close(_in->GetFd());
		delete _in;
	}
	if (_out)
	{
		DDEBUG("HTTPContext") << "  -> Deleting out-pipe fd=" << _out->GetFd();
		MulObj->DeleteFromEpoll(_out);
		close(_out->GetFd());
		delete _out;
	}
	delete []_buf;
}

bool Cgi::CanUsePipe0()
{
	bool res = (_statusfd & ePipe0);
	_statusfd &= ~ePipe0;
	return res;
}
bool Cgi::CanUsePipe1()
{
	bool res = (_statusfd & ePipe1);
	_statusfd &= ~ePipe1;
	return res;
}

string &Cgi::getStatusCode()
{
	return _cgireq.getStatusCode();
}

CgiRequest &Cgi::getCgiReq()
{
	return (_cgireq);
}

bool Cgi::isComplete()
{
	return _status == eComplete;
}
