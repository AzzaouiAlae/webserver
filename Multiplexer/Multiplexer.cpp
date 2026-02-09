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
			clientSock = accept4(fds[fdIdx], NULL, NULL, 0);
			if (clientSock != -1)
			{
				fds.push_back(Fd(clientSock, Fd::eClient, Fd::eRequest));
			}
		}
		else
		{
			memset(buff, 0, BUFF_SIZE);
			if (fds[fdIdx].routing.ReadyToSend())
			{
				int len = fds[fdIdx].routing.SendResponse(fds[fdIdx]);
				if (len > 0)
					fds[fdIdx].UpdateTime();
				else if (fds[fdIdx].isTimeOut())
				{
					fds.erase(fds.begin() + fdIdx);
					fdIdx--;
				}
			}
			else if ((len = recv(fds[fdIdx], buff, BUFF_SIZE, 0)) != -1)
			{
				cout << buff << "\n";

				if (fds[fdIdx].req.isComplete(buff))
				{
					std::cout << "Parsing of The Request is Complete\n";
					for (map<string, string>::iterator it = fds[fdIdx].req.getrequestenv().begin(); it != fds[fdIdx].req.getrequestenv().end(); it++)
					{
						cout << "|" << it->first << ":" << it->second << "|" << endl;
					}
				}
				// if (len < BUFF_SIZE)
				// {
				// 	if (fds[fdIdx].req.isComplete())
				// 	{
				// 		fds[fdIdx].routing.CreateResponse(fds[fdIdx].req);
				// 	}
				// }
			}
		}
		fdIdx++;
		if (fdIdx == (int)fds.size())
			fdIdx = 0;
	}
}