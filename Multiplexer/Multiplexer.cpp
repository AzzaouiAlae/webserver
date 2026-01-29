#include "Multiplexer.hpp"
#include "../Routing/Routing.hpp"
#include "../Fd/Fd.hpp"

void Multiplexer::MainLoop()
{
	vector<int> &sockets = Singleton::GetSockets(), ClientsSock;
	vector<Fd> fds;
	for(int i = 0; i < (int)sockets.size(); i++)
	{
		fds.push_back(sockets[i]);
	}
	int len, fdIdx = 0, clientSock;
	char buff[BUFF_SIZE + 1];
	memset(buff, 0, BUFF_SIZE + 1);
	while(true)
	{
		if (fds[fdIdx].type == Fd::eSocket)
		{
			clientSock = accept4(fds[fdIdx], NULL, NULL, SOCK_CLOEXEC | SOCK_NONBLOCK);
			if (clientSock != -1)
			{
				fds.push_back(Fd(clientSock, Fd::eClient, Fd::eRequest));
			}
		}
		else
		{
			memset(buff, 0, BUFF_SIZE);
			if ((len = recv(fds[fdIdx], buff, BUFF_SIZE, SOCK_NONBLOCK)) != -1)
			{
				fds[fdIdx].req.AddBuffer(buff);
				cout << buff << "\n";
				if (len < BUFF_SIZE)
				{
					if (fds[fdIdx].req.isComplete())
					{
						fds[fdIdx].routing.CreateResponse(fds[fdIdx].req);
					}
				}
			}
		}
		fdIdx++;
		if (fdIdx == (int)fds.size())
			fdIdx = 0;
	}
}