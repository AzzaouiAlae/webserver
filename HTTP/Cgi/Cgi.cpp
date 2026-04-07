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
#include "HTTPContext.hpp"
#include "ReadCGIStrategy.hpp"
#include "WriteToPipeStrategy.hpp"
#include "WriteChunkedCGIStrategy.hpp"
#include "WriteBufferedCGIStrategy.hpp"

Cgi::Cgi(Routing *router, ClientSocket *sok) : _router(router), _req(router->GetRequest()), _sok(sok)
{
	if (APipe::GetPipe(_pipefd) == false || APipe::GetPipe(_pipe2fd) == false)
	{
		_status = eError;
		_errorCode = "500";
		_in = NULL;
		_out = NULL;
		return;
	}

	_totalWritenToPipe = 0;

	_pid = -1;
	_childErrorStatus = -1;
	_totalSendToClient = 0;

	_multiplexer = Multiplexer::GetCurrentMultiplexer();

	_out = new CGIPipe(_pipefd[1], this);
	_multiplexer->AddAsEpollOut(_out);

	_buffer = Utility::GetBuffer();
	_readBuffer = Utility::GetBuffer();

	INFO() << "Initialized new CGI Context for executable " << _router->GetPath().getLocation()->cgiPassPath;

	_availableClientBufferLen = 0;
	_totalReadFromPipe = 0;

	_in = new CGIPipe(_pipe2fd[0], this);
	_stdin = _pipefd[0];
	_multiplexer->AddAsEpollIn(_in);

	_readStrategy = new ReadCGIStrategy(_router, _sok, _buffer, _availableClientBufferLen);
	_writePipeStrategy = new WriteToPipeStrategy(_router, _out->GetFd(), _buffer, _availableClientBufferLen);
	_totalReadFromClient = _router->GetRequest().getBody().length();
	_status = eFork;
	_headerParsed = false;
	_availableCgiBufferLen = 0;

	_readPipeStrategy = new ReadPipeStrategy(
		_in->GetFd(), _readBuffer, _availableCgiBufferLen,
		_cgireq, _headerParsed, _router->GetPath().getLocation()->chunkedSend);
	_pipeEof = false;
	_writeStrategy = NULL;
	_activeSocketIn = _readStrategy->GetStatus() != AStrategy::eComplete;
	_activePipeOut = _req.getBody().length() || _readStrategy->GetStatus() == AStrategy::eComplete;
	_activeSocketOut = false;
}

string Cgi::_resolveExcPath(const string &excName)
{
	if (excName.find('/') != string::npos)
	{
		char absPath[PATH_MAX];
		if (realpath(excName.c_str(), absPath) == NULL)
			return excName;
		string s = absPath;
		return s;
	}

	const char *envPath = getenv("PATH");
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

int Cgi::_isChildError()
{
	if (_pid == -1)
		return -1;
	if (_childErrorStatus != -1)
		return _childErrorStatus;
	int status;
	if (waitpid(_pid, &status, WNOHANG) != _pid)
		return -1;

	if (WIFEXITED(status))
	{
		_childErrorStatus = WEXITSTATUS(status) ? 1 : 0;
		CGILog(DDEBUG) << "isChildError(): child exited with code " << WEXITSTATUS(status)
					   << ", mapped status=" << _childErrorStatus;
	}
	else if (WIFSIGNALED(status) || WIFSTOPPED(status))
	{
		_childErrorStatus = 1;
		CGILog(DDEBUG) << "isChildError(): child signaled/stopped, marking error";
	}
	return _childErrorStatus;
}

void Cgi::_changeDir()
{
	string dir = _router->GetPath().getFullPath().c_str();
	dir = dir.substr(0, dir.find_last_of('/'));
	CGILog(DDEBUG) << "changeDir(): switching cwd to " << dir;
	if (chdir(dir.c_str()) != 0)
		throw "500";
}

void Cgi::_closeFds(int pipe1[2], int pipe2[2])
{
	close(pipe1[0]);
	close(pipe1[1]);
	if (_writePipeStrategy)
	{
		close(pipe2[0]);
		close(pipe2[1]);
	}
}

void Cgi::_prepareChild(string &exec)
{
	CGILog(DDEBUG) << "Attempting to fork child process for CGI...";
	exec = _resolveExcPath(_router->GetPath().getLocation()->cgiPassPath);
	CGILog(DDEBUG) << "createChild(): resolved executable path: " << exec;
	if (access(exec.c_str(), F_OK | X_OK) != 0)
	{
		CGILog(DDEBUG) << "createChild(): executable not accessible, setting 500";
		_status = eError;
		_errorCode = "500";
	}
}

void Cgi::_execChild(string &exec)
{
	Environment::CreateEnv(_req.getrequestenv());
	_changeDir();
	dup2(_stdin, 0);
	dup2(_pipe2fd[1], 1);
	_closeFds(_pipefd, _pipe2fd);
	char *execArgs[] =
		{
			(char *)exec.c_str(),
			(char *)_router->GetPath().getFullPath().c_str(),
			NULL};
	execve(exec.c_str(), execArgs, environ);
	throw "500";
}

void Cgi::_createChild()
{
	string exec;
	_prepareChild(exec);
	if (_status == eError)
		return;
	_pid = fork();
	if (_pid < 0)
	{
		CGILog(DDEBUG) << "createChild(): fork failed, setting 500";
		_status = eError;
		_errorCode = "500";
		close(_stdin);
		close(_pipe2fd[1]);
		return;
	}
	else if (_pid == 0)
		_execChild(exec);
	close(_stdin);
	close(_pipe2fd[1]);
	CGILog(DDEBUG) << "createChild(): fork succeeded, child pid=" << _pid;
	INFO() << "Successfully forked CGI child process.";
	_status = eHandleClient;
	CGILog(DDEBUG) << "createChild(): status -> eHandleClient";
}

void Cgi::_handleStrategyStatus(AStrategy *strategy)
{
	if (strategy->GetStatus() == AStrategy::eMaxBodySizeExceeded)
	{
		_status = Cgi::eError;
		_errorCode = "413";
		CGILog(DDEBUG) << "readFromClient(): max body exceeded, status -> eError(413)";
	}
	else if (strategy->GetStatus() == AStrategy::eChunkedError)
	{
		_status = Cgi::eError;
		_errorCode = "400";
		CGILog(DDEBUG) << "readFromClient(): chunked parse error, status -> eError(400)";
	}
	else if (strategy->GetStatus() == AStrategy::eReadError)
	{
		_status = Cgi::eError;
		_errorCode = "502";
		CGILog(DDEBUG) << "readFromClient(): read error, status -> eError(502)";
	}
	else if (strategy->GetStatus() == AStrategy::eWriteError)
	{
		_status = Cgi::eError;
		_errorCode = "502";
		CGILog(DDEBUG) << "sendBuffToPipe(): write error, status -> eError(502)";
	}
}

void Cgi::_readFromClient()
{
	int len = _availableClientBufferLen;
	_readStrategy->Execute();
	CGILog(DDEBUG) << "readFromClient(): len=" << _availableClientBufferLen
				   << ", strategy_status=" << _readStrategy->GetStatus();
	if (_availableClientBufferLen || _req.getBody().length() || _readStrategy->GetStatus() == AStrategy::eComplete)
	{
		if (_out)
		{
			_activePipeOut = true;
			_activeSocketIn = false;
		}
		CGILog(DDEBUG) << "readFromClient(): data read, switching to EPOLLOUT to send to CGI";
	}
	_handleStrategyStatus(_readStrategy);
	if (_availableClientBufferLen - len > 0)
		_totalReadFromClient += _availableClientBufferLen - len;

	CGILog(DDEBUG) << "readFromClient(): _totalReadFromClient=" << _totalReadFromClient;
}

void Cgi::_sendBuffToPipe()
{
	CGILog(DDEBUG) << "sendBuffToPipe(): len=" << _availableClientBufferLen;
	int len = _availableClientBufferLen;
	_writePipeStrategy->Execute();
	_totalWritenToPipe += len - _availableClientBufferLen;
	CGILog(DDEBUG) << "sendBuffToPipe(): len=" << _availableClientBufferLen
				   << ", strategy_status=" << _writePipeStrategy->GetStatus();
	_handleStrategyStatus(_writePipeStrategy);
	if (_readStrategy->GetStatus() == AStrategy::eComplete && 
	_availableClientBufferLen == 0 && _req.getBody().length() == 0)
	{
		_multiplexer->ScheduleForDeletion(_out);
		_out = NULL;
	}
	else if (_availableClientBufferLen == 0 && 
		_readStrategy->GetStatus() != AStrategy::eComplete)
		_activeSocketIn = true;
	else if (_availableClientBufferLen != 0 || _req.getBody().length() != 0)
		_activePipeOut = true;
	else
		_activePipeOut = false;
}

void Cgi::_activeFds()
{
    if (_activeSocketIn && _activeSocketOut)
        _multiplexer->ChangeToEpollInOut(_sok);
	else if (_activeSocketIn)
		_multiplexer->ChangeToEpollIn(_sok);
    else if (_activeSocketOut)
        _multiplexer->ChangeToEpollOut(_sok);
    else
        _multiplexer->ChangeToEpollOneShot(_sok);
    
    if (_in)
		_multiplexer->ChangeToEpollIn(_in);
	if (_out && _activePipeOut)
		_multiplexer->ChangeToEpollOut(_out);
}

void Cgi::_readFromCGI()
{
	_readPipeStrategy->Execute();
	CGILog(DDEBUG) << "readFromCGI(): len=" << _availableCgiBufferLen
				   << ", strategy_status=" << _readPipeStrategy->GetStatus();
	_handleStrategyStatus(_readPipeStrategy);
	if (_headerParsed)
		_activeSocketOut = true;
}

void Cgi::_errorHandler()
{
	CGILog(DDEBUG) << "ErrorHandler(): totalSendToClient=" << _totalSendToClient;
	if (_totalSendToClient == 0)
	{
		_status = Cgi::eError;
		_errorCode = "502";
		CGILog(DDEBUG) << "ErrorHandler(): status -> eError(502)";
	}
	else
	{
		_status = eComplete;
		CGILog(DDEBUG) << "ErrorHandler(): status -> eComplete";
	}
}

void Cgi::_writeToClient()
{
	if (_writeStrategy == NULL)
	{
		if (_router->loc->chunkedSend)
			_writeStrategy = new WriteChunkedCGIStrategy(_sok, _readBuffer, _availableCgiBufferLen, _cgireq, _pipeEof);
		else
			_writeStrategy = new WriteBufferedCGIStrategy(_sok, _readBuffer, _availableCgiBufferLen, _cgireq, _pipeEof);
	}
	if (_readPipeStrategy->GetStatus() <= AStrategy::eComplete)
		_pipeEof = true;
	_writeStrategy->Execute();
	_handleStrategyStatus(_writeStrategy);
	if (_writeStrategy->GetStatus() == AStrategy::eComplete)
	{
		_status = eComplete;
		CGILog(DDEBUG) << "writeToClient(): write complete, status -> eComplete";
	}
	if (_availableCgiBufferLen == 0 && !_writeStrategy->IsBusy())
	{
		_activeSocketOut = false;
	}
}

void Cgi::Handle()
{
	CGILog(DDEBUG) 
		<< "Handle() called. Current status: " << _status
		<< "\nevent on socket fd=" << _sok->GetEvents()
		<< "\navailableClientBufferLen=" << _availableClientBufferLen
		<< "\navailableCgiBufferLen=" << _availableCgiBufferLen
		<< "\nheaderParsed=" << _headerParsed
		<< "\npipeEof=" << _pipeEof
		<< "\nreadStrategyStatus=" << _readStrategy->GetStatus()
		<< "\n_headerParsed=" << _headerParsed;

	if (_status >= eComplete)
		return;
	if (_status == eFork)
		_createChild();
	else
	{
		if (_sok->GetEvents() & EPOLLIN)
			_readFromClient();
		if (_sok->GetEvents() & EPOLLOUT)
			_writeToClient();
		
	}
	if (_isChildError() == 1)
		_errorHandler();
	_activeFds();
}

void Cgi::SetStateByFd(int fd)
{
	(void)fd;

	CGILog(DDEBUG) << "SetStateByFd(): event on fd=" << fd;
	
	if (_in && fd == _in->GetFd())
	{
		CGILog(DDEBUG) << "SetStateByFd(): dispatching readFromCGI()";
		_readFromCGI();
	}
	else if (_out && fd == _out->GetFd())
	{
		CGILog(DDEBUG) << "SetStateByFd(): dispatching sendBuffToPipe()";
		_sendBuffToPipe();
	}
	_activeFds();
}

Cgi::~Cgi()
{
	CGIPipe *inPipe = dynamic_cast<CGIPipe *>(_in);
	if (inPipe)
	{
		inPipe->CgiClosed();
		_multiplexer->ScheduleForDeletion(_in);
		DDEBUG("HTTPContext") << "  -> Deleting in-pipe fd=" << _in->GetFd();
	}
	CGIPipe *outPipe = dynamic_cast<CGIPipe *>(_out);
	if (outPipe)
	{
		outPipe->CgiClosed();
		_multiplexer->ScheduleForDeletion(_out);
		DDEBUG("HTTPContext") << "  -> Deleting out-pipe fd=" << _out->GetFd();
	}
	if (_pid > 0)
	{
		if (_isChildError() == -1)
		{
			INFO() << "Child process still running. Sending SIGKILL to PID: " << _pid;
			kill(_pid, SIGKILL);
			waitpid(_pid, NULL, 0);
		}
	}
	else if (_isChildError() == 0)
		INFO() << "Child process already terminated with status: " << (_isChildError() == 1 ? "Error" : "Success");
	Utility::ReleaseBuffer(_buffer);
	Utility::ReleaseBuffer(_readBuffer);
	delete _readStrategy;
	delete _writePipeStrategy;

	if (_readPipeStrategy)
		delete _readPipeStrategy;
	if (_writeStrategy)
		delete _writeStrategy;
}

bool Cgi::isComplete()
{
	return _status == eComplete;
}

int Cgi::getStatus() const
{
	return _status;
}

string Cgi::getErrorCode() const
{
	return _errorCode;
}

void Cgi::PipeClosed(int fd)
{
	if (_in && fd == _in->GetFd())
	{
		CGILog(DDEBUG) << "PipeClosed(): input pipe closed, marking EOF";
		_pipeEof = true;
		_in = NULL;
		if (_sok->MarkedToDelete == false)
			_multiplexer->ChangeToEpollOut(_sok);
	}
	else if (_out && fd == _out->GetFd())
	{
		CGILog(DDEBUG) << "PipeClosed(): output pipe closed, marking EOF and scheduling for deletion";
		_out = NULL;
	}
}