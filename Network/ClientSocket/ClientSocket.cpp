#include "ClientSocket.hpp"
#include "HTTPContext.hpp"

void ClientSocket::Handle()
{
	if (MarkedToDelete)
		return;
	context->Handle(this);
	ConnectionContext::HandleKeepAlive(this);
}

ClientSocket::ClientSocket(int fd, int timeout) : ISocket(fd, "ClientSocket")
{
	_timeoutStatus = NetIO::eNotTimedOut;
	_timeout = timeout;
	_isKeepAlive = false;
	DEBUG("ClientSocket") << "ClientSocket initialized, fd=" << fd;
	_lastTime = Utility::CurrentTime();
	_closeConnection = false;
	_sendStart = false;
}

ClientSocket::~ClientSocket()
{
	delete context;
}

bool ClientSocket::IsTimeOut()
{
	time_t now = Utility::CurrentTime();
	if (GetEndTime() < now || _timeoutStatus == NetIO::eTimedOut)
	{
		if (_timeoutStatus == NetIO::eNotTimedOut || _timeoutStatus == NetIO::eTimedOut)
			_timeoutStatus = NetIO::eSendTimeout;
		return true;
	}
	return false;
}

bool ClientSocket::ClosedConnection() const
{
	return _closeConnection;
}

void ClientSocket::SetCloseConnection(bool val)
{
	_closeConnection = val;
}

time_t ClientSocket::GetEndTime() const
{
	return _lastTime + _timeout;
}

void ClientSocket::SetKeepAlive(bool val)
{
	_isKeepAlive = val;
}

bool ClientSocket::IsKeepAlive() const
{
	return _isKeepAlive;
}

void ClientSocket::SetTimeout(int timeout)
{
	_timeout = timeout;
	NetIO::AddToTimeoutList(this);
}

int ClientSocket::GetTimeout() const
{
	return _timeout;
}

void ClientSocket::UpdateTime()
{
	if (_timeoutStatus == NetIO::eMarkedForDeletion)
		return;
	if (_timeoutStatus == NetIO::eTimedOut)
		_timeoutStatus = NetIO::eMarkedForDeletion;
	_lastTime = Utility::CurrentTime();
	NetIO::AddToTimeoutList(this);
}

int ClientSocket::GetTimeoutStatus() const
{
	return _timeoutStatus;
}

void ClientSocket::SetTimeoutStatus(int status)
{
	_timeoutStatus = status;
}

void ClientSocket::SetSendStart(bool val)
{
	_sendStart = val;
}

bool ClientSocket::GetSendStart() const
{
	return _sendStart;
}