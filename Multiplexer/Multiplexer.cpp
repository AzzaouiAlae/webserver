#include "Multiplexer.hpp"

void Multiplexer::MainLoop()
{
	vector<int> &sockets = Singleton::GetSockets(), ClientsSock;
	int sockIdx = 0, ClientIdx = 0, len;
	char buff[BUFF_SIZE + 1];
	memset(buff, 0, BUFF_SIZE + 1);
	while(true)
	{
		int clientSock = accept4(sockets[sockIdx], NULL, NULL, SOCK_CLOEXEC | SOCK_NONBLOCK);
		sockIdx++;
		if (sockIdx == sockets.size())
			sockIdx = 0;
		if (clientSock != -1)
		{
			// should create a request object
			ClientsSock.push_back(clientSock);
		}
		memset(buff, 0, BUFF_SIZE );
		if ((len = recv(ClientsSock[ClientIdx], buff, BUFF_SIZE, SOCK_NONBLOCK)) != -1)
		{
			//should send the buffer to request obj to add it to the request buffer
			//when the HTTP header complete start add on the body (body is a seperate string) 
			cout << buff << "\n";
			if (len < BUFF_SIZE)
			{
				// is read request complete ??
				// if the request method and has /n/r
				// if post check the request 
				// Get request --> prosses this request --> send respense
				// request obj most has a (enum status) and (long time) 
			}
		}
		ClientIdx++;
		if (ClientIdx == ClientsSock.size())
			ClientIdx = 0;
	}
}