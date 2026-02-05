#include "../Unity-master/src/unity.h"
#include "../../Socket/socket.hpp"


void setUp(void)
{}

void tearDown(void)
{
	vector<int>& sockets = Singleton::GetSockets();
	for(int i = 0; i < (int)sockets.size(); i++)
	{
		close(sockets[i]);
	}
	sockets.clear();
}

void Test1()
{
	//arrnge
	string host = "";
	string port = "3007";
	vector<int>& Socktes = Singleton::GetSockets();
	SockStatus statusExp = eSockCreated;
	int sizeExp = 1;

	//act
	SockStatus status = SocketIO::AddSocket(host, port);

	//assert
	TEST_ASSERT_EQUAL_INT32(statusExp, status);
	TEST_ASSERT_EQUAL_INT32(sizeExp, (int)Socktes.size());
}

void Test2()
{
	//arrnge
	string host = "127.0.0.1";
	string port = "3008";
	vector<int>& Socktes = Singleton::GetSockets();
	SockStatus statusExp = eSockCreated;
	int sizeExp = 1;

	//act
	SockStatus status = SocketIO::AddSocket(host, port);

	//assert
	TEST_ASSERT_EQUAL_INT32(statusExp, status);
	TEST_ASSERT_EQUAL_INT32(sizeExp, (int)Socktes.size());
}

void Test3()
{
	//arrnge
	string host = "127.0.0.1";
	string port = "0003008434";
	vector<int>& Socktes = Singleton::GetSockets();
	SockStatus statusExp = eGetaddrinfo;
	int sizeExp = 0;

	//act
	SockStatus status = SocketIO::AddSocket(host, port);

	//assert
	TEST_ASSERT_EQUAL_INT32(statusExp, status);
	TEST_ASSERT_EQUAL_INT32(sizeExp, (int)Socktes.size());
}

void Test4()
{
	//arrnge
	string host = "127.0.0.1";
	string port = "80";
	vector<int>& Socktes = Singleton::GetSockets();
	SockStatus statusExp = eCanNotBindAnySocket;
	int sizeExp = 1;

	//act
	SockStatus status = SocketIO::AddSocket(host, port);
	status = SocketIO::AddSocket(host, port);

	//assert
	TEST_ASSERT_EQUAL_INT32(statusExp, status);
	TEST_ASSERT_EQUAL_INT32(sizeExp, (int)Socktes.size());
}

void Test5()
{
	//arrnge
	string host = "[::1]";
	string port = "1337";
	vector<int>& Socktes = Singleton::GetSockets();
	SockStatus statusExp = eSockCreated;
	int sizeExp = 1;

	//act
	SockStatus status = SocketIO::AddSocket(host, port);

	//assert
	TEST_ASSERT_EQUAL_INT32(statusExp, status);
	TEST_ASSERT_EQUAL_INT32(sizeExp, (int)Socktes.size());
}

void Test6()
{
	//arrnge
	string host = "[::1]";
	string port = "1337a";
	vector<int>& Socktes = Singleton::GetSockets();
	SockStatus statusExp = eGetaddrinfo;
	int sizeExp = 0;

	//act
	SockStatus status = SocketIO::AddSocket(host, port);

	//assert
	TEST_ASSERT_EQUAL_INT32(statusExp, status);
	TEST_ASSERT_EQUAL_INT32(sizeExp, (int)Socktes.size());
}


void Test7()
{
	//arrnge
	string host = "[::1]";
	string port = "65538"; 
	vector<int>& Socktes = Singleton::GetSockets();
	SockStatus statusExp = eGetaddrinfo;
	int sizeExp = 0;

	//act
	SockStatus status = SocketIO::AddSocket(host, port);

	//assert
	TEST_ASSERT_EQUAL_INT32(statusExp, status);
	TEST_ASSERT_EQUAL_INT32(sizeExp, (int)Socktes.size());
}

void Test8()
{
	//arrnge
	string host = "[::1]aa";
	string port = "655"; 
	vector<int>& Socktes = Singleton::GetSockets();
	SockStatus statusExp = eGetaddrinfo;
	int sizeExp = 0;

	//act
	SockStatus status = SocketIO::AddSocket(host, port);

	//assert
	TEST_ASSERT_EQUAL_INT32(statusExp, status);
	TEST_ASSERT_EQUAL_INT32(sizeExp, (int)Socktes.size());
}

int main()
{
	RUN_TEST(Test1);
	RUN_TEST(Test2);
	RUN_TEST(Test3);
	RUN_TEST(Test4);
	RUN_TEST(Test5);
	RUN_TEST(Test6);
	RUN_TEST(Test7);
	RUN_TEST(Test8);

	return (UnityEnd());
}