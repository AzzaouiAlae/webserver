#include "Headers.hpp"
#include "Socket/Socket.hpp"

void sigpipe_handler(int signum) 
{
	(void)signum;
    Utility::SigPipe = true;
	
}

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	Logging log("webserver.logs");
	(void)log;
	
	signal(SIGPIPE, sigpipe_handler);
	string filename;
	if (argc < 2)
		filename = DEFAULT_CONF;
	else {
		filename = argv[1];
	}
	
	try
	{
		
		Tokenizing token(filename);
		token.split_tokens();
		

		
		Validation valid(token.get_tokens());
		valid.CheckValidation();
		
		
		
		Multiplexer m;
		m.MainLoop();
		
	}
	catch (const std::exception &e)
	{
		

		set<AFd *> &fds = Singleton::GetFds();
		set<AFd *>::iterator it = fds.begin();

		for (; it != fds.end(); it = fds.begin()) {
			delete *it;
		}
		return 1;
	}
	
}
