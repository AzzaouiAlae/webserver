#include "NetIO.hpp"
#include "HTTPContext.hpp"

priority_queue<NetIO::TimeoutEntry, vector<NetIO::TimeoutEntry>, greater<NetIO::TimeoutEntry> > NetIO::timeoutList;


int NetIO::Send(void *buff, int size, int fd)
{
	ssize_t sent = send(fd, buff, size, MSG_NOSIGNAL);
	return sent;
}

int NetIO::BuffToPipe(void *buff, int size, int pipefd)
{
	struct iovec iov;
	iov.iov_base = buff;
	iov.iov_len = size;

	ssize_t n = vmsplice(pipefd, &iov, 1, SPLICE_F_NONBLOCK);
	return n;
}

int NetIO::PipeToSock(int inputfd, size_t size, int socketFd)
{
	ssize_t sent = splice(inputfd, NULL, socketFd, NULL, size, SPLICE_F_NONBLOCK);
	return sent;
}

ssize_t NetIO::FileToSocket(int fileFd, int size, int socketFd)
{
	ssize_t len = sendfile(socketFd, fileFd, NULL, size);
	return len;
}

int NetIO::PipeToFile(int fileFD, int inputfd, int size)
{
	ssize_t len = splice(inputfd, NULL, fileFD, NULL, size, 0);
	return len;
}

int NetIO::SocketToPipe(int size, int socketFd, int pipefd)
{
	int len = splice(socketFd, NULL, pipefd, NULL, size, 0);
	return len;
}

void NetIO::CloseTheOldestSocket()
{
	if (timeoutList.empty())
		return;
	ClientSocket *sock;
	ClearIncorrectTimeout();
	if (!timeoutList.empty())
	{
		sock = timeoutList.top().sock;
		Multiplexer::GetCurrentMultiplexer()->ScheduleForDeletion(sock);
		sock->SetKeepAlive(false);
		timeoutList.pop();
		DDEBUG("NetIO") << "  -> Closed oldest socket fd=" << sock->GetFd() << " due to timeout.";
	}
}

long NetIO::GetSocketTimeout()
{
	ClearIncorrectTimeout();
	if (timeoutList.empty())
		return 20000;
	ClientSocket *sock = timeoutList.top().sock;
	long remainingTime = sock->GetEndTime() - Utility::CurrentTime();
	if (remainingTime < 0)
		return 1;
	return remainingTime / 1000;
}

void NetIO::ClearIncorrectTimeout()
{
	ClientSocket *sock;
	set<AFd *> &fds = Singleton::GetFds();

	while (timeoutList.empty() == false)
	{
		sock = timeoutList.top().sock;
		if (fds.find(sock) == fds.end() || sock->MarkedToDelete ||
			sock->GetEndTime() != timeoutList.top().snapshotTime)
		{
			DDEBUG("NetIO") << "  -> Socket not found in active fds, removing from timeoutList.";
			timeoutList.pop();
		}
		else
			break;
	}
}

void NetIO::ClearTimeout()
{
	DDEBUG("NetIO") << "Clearing timeouts, current timeoutList size=" << timeoutList.size();
	time_t now = Utility::CurrentTime();
	ClientSocket *sock;
	ClearIncorrectTimeout();
	while (timeoutList.empty() == false)
	{
		sock = timeoutList.top().sock;
		if (sock->GetEndTime() < now)
		{
			if (sock->GetTimeoutStatus() >= NetIO::eTimedOut)
			{
				Multiplexer::GetCurrentMultiplexer()->ScheduleForDeletion(sock);
				timeoutList.pop();
				DDEBUG("NetIO") << "  -> Socket fd=" << sock->GetFd() << " timed out and marked for deletion.";
			}
			else
			{
				DDEBUG("NetIO") << "  -> Socket fd=" << sock->GetFd() << " timed out, marking for timeout and will check again on next clearTimeout.";
				sock->UpdateTime();
				if (sock->GetSendStart())
					Multiplexer::GetCurrentMultiplexer()->ScheduleForDeletion(sock);
				else
					sock->SetTimeoutStatus(NetIO::eTimedOut);
				Multiplexer::GetCurrentMultiplexer()->ChangeToEpollOut(sock);
				timeoutList.pop();
			}
		}
		break;
	}
}

void NetIO::AddToTimeoutList(ClientSocket *sock)
{
	DDEBUG("NetIO") << "Adding socket fd=" << sock->GetFd() << " to timeoutList with endTime=" << sock->GetEndTime();
	TimeoutEntry entry;
	entry.sock = sock;
	entry.snapshotTime = sock->GetEndTime();
	timeoutList.push(entry);
}