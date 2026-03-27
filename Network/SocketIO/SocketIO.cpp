#include "SocketIO.hpp"
#include "HTTPContext.hpp"

priority_queue<SocketIO::TimeoutEntry, vector<SocketIO::TimeoutEntry>, greater<SocketIO::TimeoutEntry> > SocketIO::timeoutList;

void SocketIO::Handle()
{
	context->Handle(this);
}

void SocketIO::SetStateByFd(int fd)
{
	if (pipeInitialized)
	{
		if (fd == pipefd[0])
			status |= ePipe0;
		else if (fd == pipefd[1])
			status |= ePipe1;
		else if (this->fd == fd)
			status |= eSocket;
	}
	else if (this->fd == fd)
		status |= eSocket;
}

bool SocketIO::clearPipe()
{
	if ((status & ePipe0) == 0)
		return false;
	char *buff = Utility::GetBuffer();
	ssize_t len = read(pipefd[0], buff, BUF_SIZE);
	Utility::ReleaseBuffer(buff);
	if (len > 0)
		return true;
	return false;
}

int SocketIO::Send(void *buff, int size)
{
	ssize_t sent = send(fd, buff, size, MSG_NOSIGNAL);
	if (sent <= 0)
		errorNumber = eWriteError;
	return sent;
}

int SocketIO::SendBuffToPipe(void *buff, int size, bool usePending)
{
	if ((status & ePipe1) == 0)
		return 0;
	struct iovec iov;
	iov.iov_base = buff;
	iov.iov_len = size;

	ssize_t n = vmsplice(pipefd[1], &iov, 1, SPLICE_F_NONBLOCK);
	if (n <= 0)
	{
		ERR() << "Socket fd: " << fd << " SendBuffToPipe";
		errorNumber = ePipe1Error;
	}
	status &= ~ePipe1;
	if (n <= 0)
		return -1;
	if (usePending)
		pendingInPipe += n;
	return n;
}

int SocketIO::SendPipeToSock(int inputfd, size_t size)
{
	ssize_t sent = splice(inputfd, NULL, this->fd, NULL, size, SPLICE_F_NONBLOCK);
	if (sent <= 0)
	{
		ERR() << "Socket fd: " << fd << " SendPipeToSock";
		errorNumber = eWriteError;
	}
	return sent;
}

ssize_t SocketIO::FileToSocket(int fileFd, int size)
{
	ssize_t len = sendfile(this->fd, fileFd, NULL, size);
	if (len <= 0)
	{
		ERR() << "Socket fd: " << fd << " FileToSocket eWriteError";
		errorNumber = eWriteError;
	}
	return len;
}

int SocketIO::PipeToFile(int fileFD, int inputfd, int size)
{
	ssize_t len = splice(inputfd, NULL, fileFD, NULL, size, 0);
	if (len <= 0)
		errorNumber = ePipe0Error;
	return len;
}

int SocketIO::SocketToFile(int fileFD, int size)
{
	int len = 0, flag = (eSocket | ePipe1);
	DDEBUG("SocketIO") << "SocketToFile: fd=" << this->fd << ", pendingInPipe=" << pendingInPipe;

	if ((status & flag) == flag && pendingInPipe < size)
	{
		len = splice(this->fd, NULL, pipefd[1], NULL, size - pendingInPipe, 0);
		status &= ~flag;
		if (len > 0)
			pendingInPipe += len;
		else
		{
			ERR() << "Socket fd: " << fd << " SocketToFile eReadError";
			errorNumber = eReadError;
		}
		len = 0;
	}
	if ((status & ePipe0) && pendingInPipe > 0)
	{
		len = splice(pipefd[0], NULL, fileFD, NULL, pendingInPipe, 0);
		status &= ~ePipe0;
		if (len <= 0)
		{
			ERR() << "Socket fd: " << fd << " SocketToFile ePipe0Error";
			errorNumber = ePipe0Error;
		}
		pendingInPipe -= len;
	}
	DDEBUG("SocketIO") << "SocketToFile: fd=" << this->fd << ", pendingInPipe=" << pendingInPipe << ", written=" << len;
	return len;
}

int SocketIO::SendSocketToPipe(int size, bool usePending)
{
	int len = 0;
	if (status & ePipe1)
	{
		len = splice(fd, NULL, pipefd[1], NULL, size, 0);
		if (len <= 0)
		{
			ERR() << "Socket fd: " << fd << " SendSocketToPipe eReadError";
			errorNumber = eReadError;
		}
		status &= ~ePipe1;
		if (len == -1)
			return -1;
		if (usePending)
			pendingInPipe += len;
		DDEBUG("SocketIO") << "SendSocketToPipe: fd=" << fd << ", spliced=" << len << ", pendingInPipe=" << pendingInPipe;
	}
	return len;
}

SocketIO::SocketIO(int fd, int timeout) : ISocket(fd, "SocketIO"), pipeInitialized(false), pendingInPipe(0), status(0), errorNumber(0)
{
	_timeoutStatus = eNotTimedOut;
	_timeout = timeout;
	_isKeepAlive = false;
	buff = NULL;
	DEBUG("SocketIO") << "SocketIO initialized, fd=" << fd;
	lastTime = Utility::CurrentTime();
	timeoutList.push((TimeoutEntry){this, GetEndTime()});
	closeConnection = false;
}

void SocketIO::setupPipes()
{
	if (APipe::GetPipe(pipefd))
	{
		pipeInitialized = true;
		DDEBUG("SocketIO") << "SocketIO created fd=" << fd << ", new pipe [" << pipefd[0] << ", " << pipefd[1] << "]";
	}
	else
	{
		MarkedToDelete = true;
	}
}

SocketIO::~SocketIO()
{
	DDEBUG("SocketIO") << "SocketIO destructor, fd=" << this->fd << ", pendingInPipe=" << pendingInPipe;
	if (!_isKeepAlive || isTimeOut())
	{
		int closeResult = close(fd);
		INFO() << "Socket fd: " << this->fd << " closed,  closeResult: " << closeResult;
	}
	delete context;
	if (pipeInitialized && pendingInPipe == 0)
	{
		APipe::ReleasePipe(pipefd);
		DDEBUG("SocketIO") << "  -> Released pipe back to pool [" << pipefd[0] << ", " << pipefd[1] << "]";
	}
	else if (pipeInitialized)
	{
		close(pipefd[0]);
		close(pipefd[1]);
		DDEBUG("SocketIO") << "  -> Closed pipe [" << pipefd[0] << ", " << pipefd[1] << "]";
	}
	Singleton::GetFds().erase(this);
}

bool SocketIO::isTimeOut()
{
	time_t now = Utility::CurrentTime();
	if (GetEndTime() < now)
	{
		if (_timeoutStatus == eNotTimedOut)
			_timeoutStatus = eTimedOut;
		return true;
	}
	return false;
}

int SocketIO::GetTimeoutStatus() 
{
	return _timeoutStatus;
}

void SocketIO::closeTheOldestSocket()
{
	if (timeoutList.empty())
		return;
	set<AFd *> &fds = Singleton::GetFds();
	SocketIO *sock;
	while (!timeoutList.empty())
	{
		sock = timeoutList.top().sock;
		if (fds.find(sock) == fds.end() || sock->MarkedToDelete == true || sock->GetEndTime() != timeoutList.top().snapshotTime)
		{
			DDEBUG("SocketIO") << "  -> Socket not found in active fds, removing from timeoutList.";
			timeoutList.pop();
		}
		else
		{
			Multiplexer::ClearObj(sock);
			sock->_isKeepAlive = false;
			timeoutList.pop();
			DDEBUG("SocketIO") << "  -> Closed oldest socket fd=" << sock->GetFd() << " due to timeout.";
			break;
		}
	}
}

void SocketIO::clearTimeout()
{
	DDEBUG("SocketIO") << "Clearing timeouts, current timeoutList size=" << timeoutList.size();
	time_t now = Utility::CurrentTime();
	set<AFd *> &fds = Singleton::GetFds();
	SocketIO *sock;
	while (timeoutList.empty() == false)
	{
		sock = timeoutList.top().sock;
		if (fds.find(sock) == fds.end() || sock->MarkedToDelete == true ||
			sock->GetEndTime() != timeoutList.top().snapshotTime)
		{
			DDEBUG("SocketIO") << "  -> Socket not found in active fds, removing from timeoutList.";
			timeoutList.pop();
		}
		else if (sock->GetEndTime() < now)
		{
			if (sock->GetTimeoutStatus() >= eTimedOut)
			{
				Multiplexer::ClearObj(sock);
				timeoutList.pop();
				DDEBUG("SocketIO") << "  -> Socket fd=" << sock->GetFd() << " timed out and marked for deletion.";
			}
			else
			{
				DDEBUG("SocketIO") << "  -> Socket fd=" << sock->GetFd() << " timed out, marking for timeout and will check again on next clearTimeout.";
				sock->_timeoutStatus = eTimedOut;
				Multiplexer::GetCurrentMultiplexer()->ChangeToEpollOut(sock);
				break;
			}
		}
		else
			break;
	}
}

time_t SocketIO::GetEndTime() const
{
	return lastTime + _timeout;
}

void SocketIO::setKeepAlive(bool val)
{
	_isKeepAlive = val;
}

bool SocketIO::isKeepAlive() const
{
	return _isKeepAlive;
}

void SocketIO::setTimeout(int timeout)
{
	_timeout = timeout;
	timeoutList.push((TimeoutEntry){this, GetEndTime()});
}

int SocketIO::getTimeout() const
{
	return _timeout;
}

void SocketIO::UpdateTime()
{
	if (_timeoutStatus == eMarkedForDeletion)
		return;
	if (_timeoutStatus == eTimedOut)
	{
		_timeoutStatus = eMarkedForDeletion;
		lastTime = Utility::CurrentTime() - (_timeout / 2);
	}
	else
		lastTime = Utility::CurrentTime();
	timeoutList.push((TimeoutEntry){this, GetEndTime()});
}

bool SocketIO::CanUsePipe0()
{
	bool res = (status & ePipe0);
	status &= ~ePipe0;
	return res;
}

bool SocketIO::CanUsePipe1()
{
	bool res = (status & ePipe1);
	status &= ~ePipe1;
	return res;
}