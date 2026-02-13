#include "Headers.hpp"
#include "Socket/Socket.hpp"

void sigpipe_handler(int signum) 
{
	(void)signum;
    Utility::SigPipe = true;
	Logging::Warn() << "sigpipe has occurred";
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
	Logging::Info() << "Server start with configuation file: " << filename << "";
	try
	{
		Logging::Debug() << "Tokenizing start";
		Tokenizing token(filename);
		token.split_tokens();
		Logging::Debug() << "Tokenizing complete";

		Logging::Debug() << "Validation and parsing start";
		Validation valid(token.get_tokens());
		valid.CheckValidation();
		Logging::Debug() << "Validation and parsing complete";

		Logging::Debug() << "The main loop start";
		Multiplexer m;
		m.MainLoop();
		Logging::Debug() << "The main loop stop";
	}
	catch (const std::exception &e)
	{
		Logging::Error() << e.what();

		set<AFd *> &fds = Singleton::GetFds();
		set<AFd *>::iterator it = fds.begin();

		for (; it != fds.end(); it = fds.begin()) {
			delete *it;
		}
		return 1;
	}
	
}
