/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aazzaoui <aazzaoui@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/14 01:39:07 by oel-bann          #+#    #+#             */
/*   Updated: 2026/03/07 04:36:57 by aazzaoui         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cgi.hpp"
#include "CGIPipe.hpp"
#include "AMethod.hpp"
#include <string.h>

string Cgi::resolveExcPath(const string &excName) 
{
    if (excName.find('/') != string::npos)
        return excName;
    
    const char* envPath = getenv("PATH");
    if (!envPath) 
        return excName; 

    string pathString(envPath);
    stringstream ss(pathString);
    string directory;

    while (getline(ss, directory, ':')) 
	{
        string fullPath = (directory.empty() ? "." : directory) + "/" + excName;

        if (access(fullPath.c_str(), F_OK | X_OK) == 0) 
            return fullPath;
    }
    return excName;
}

Cgi::Cgi(ClientRequest &req,  char *const *exec, SocketIO *sok) : _req(req), _sok(sok)
{
	if (APipe::GetPipe(_pipefd) == false) {
		Error::ThrowError("500");
	}
	_statusfd = 0;
	_status = eFork;
	_exec = exec;
	_reqlen = 0;
	_responseHeaderStr.clear();
	_responselen = 0;
	_buf = Utility::GetBuffer();
	_shouldSend = 0;
	_pid = -1;
	Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();

	_in = new CGIPipe(_pipefd[0], this);
	MulObj->AddAsEpollIn(_in);

	_out = new CGIPipe(_pipefd[1], this);
	MulObj->AddAsEpollOut(_out);

	INFO() << "Initialized new CGI Context for executable: " << (_exec ? _exec[0] : "UNKNOWN");
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
	CGILog(DDEBUG) << "Attempting to fork child process for CGI...";
	string exec = resolveExcPath(*_exec);
	if (access(exec.c_str(), F_OK | X_OK) != 0)
		Error::ThrowError("500");
	_pid = fork();
	if (_pid < 0)
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
	INFO() << "Successfully forked CGI child process.";
}

void Cgi::writetocgi()
{
	int len = 0;
	size_t size = _req.getcontentlen() - _reqlen;

	CGILog(DDEBUG) 
		<< "writetocgi() called. Status: " << _status 
		<< " | Remaining body size: " << size;

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
			CGILog(DDEBUG) << "Sending buffer to CGI pipe. write: " << len << " bytes.";
			if (_sok->errorNumber == ePipe1Error)
				Error::ThrowError("502");
			_reqlen += len;
			CGILog(DDEBUG) << "Wrote " << len 
				<< " bytes from buffer. Total sent: " 
				<< _reqlen << "/" << _req.getcontentlen();
			if (_reqlen == body.size()) {
				INFO() << "[CGI PID: " << _pid << "] Memory buffer fully written to CGI pipe. Transitioning to eSendSockToPipe.";
				_status = eSendSockToPipe;
			}
		}
		else if (_status == eSendSockToPipe)
		{
			CGILog(DDEBUG) << "Streaming directly from socket to CGI pipe. Target size: " << size;
			len = _sok->SendSocketToPipe(size, false);
			if (_sok->errorNumber == eReadError)
				Error::ThrowError("502");
			_reqlen += len;
			CGILog(DDEBUG) << "Streamed " << len 
				<< " bytes to pipe. Total sent: " 
				<< _reqlen << "/" << _req.getcontentlen();
		}
	}

	if (_req.getBody().length() == _req.getcontentlen() || _reqlen >= _req.getcontentlen())
	{
		CGILog(DDEBUG) << "Body fully received from client. Switching Multiplexer to EpollOut.";
		Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();
		MulObj->ChangeToEpollOut(_sok);
	}
	if (_reqlen >= _req.getcontentlen() || _req.getthereisbody() == false ) {
		INFO() << "[CGI PID: " << _pid << "] Request transmission to CGI complete. Transitioning to eReadCgiResponse.";
		_status = eReadCgiResponse;
	}
	
}

void Cgi::readfromcgi()
{
	CGILog(DDEBUG) << "readfromcgi() called.";
	int len = 0;
	if (!CanUsePipe0()) {
		Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();
		MulObj->ChangeToEpollOneShot(_sok);
		CGILog(DDEBUG) << "readfromcgi(), Can't Use Pipe 0";
		return;
	}
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
		CGILog(DDEBUG) 
			<< "Writing CGI headers to client. Header size: " 
            << _responseHeaderStr.length() << " bytes, Sent so far: " << _responselen;
		int len = 0;
		void *buff = (void *)(_responseHeaderStr.c_str() + _responselen);
		len = _sok->Send(buff, _shouldSend - _responselen);
		if (_sok->errorNumber == eWriteError)
			Error::ThrowError("502");
		_responselen += len;
		if (_responselen == _shouldSend)
			_status = eWritePipeToClient;
		}
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
			CGILog(DDEBUG) << "Sent " << len << " body bytes from buffer. Total sent: " << _responselen;
		}
	if ((_cgireq.getcontentlen() - body.length()) <= _responselen)
		_status = eComplete;
	}
}

void Cgi::Handle()
{
	CGILog(DDEBUG) << "Handle() called. Current status: " << _status;
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
	CGILog(DDEBUG) << "SetStateByFd() called. Active fd: " << fd;
	if (fd == _pipefd[0]) {
		_statusfd |= ePipe0;
		if (_status == eReadCgiResponse)
		{
			Multiplexer *MulObj = Multiplexer::GetCurrentMultiplexer();
			MulObj->ChangeToEpollOut(_sok);
		}
	}
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
	APipe::ReleasePipe(_pipefd);
	Utility::ReleaseBuffer(_buf);
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
