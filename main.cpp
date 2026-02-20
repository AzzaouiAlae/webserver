#include "Headers.hpp"
#include "Socket/Socket.hpp"
#include "DefaultPages/DefaultPages.hpp"

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
	Logging::Info() << "Server start with configuation file: " << filename << "";
	try
	{
		Logging::Debug() << "Tokenizing start";
		Tokenizing token(filename);
		token.split_tokens();
		Logging::Debug() << "Tokenizing complete";

		Logging::Debug() << "Parsing start";
		Parsing pars(token.get_tokens());
		pars.BuildAST();
		Logging::Debug() << "Parsing complete";

		Logging::Debug() << "Validation start";
		Validation val(Singleton::GetASTroot());
		val.Validate();
		Logging::Debug() << "Validation complete";


		pars.FillConf();

		DefaultPages::InitDefaultPages();
		Logging::Debug() << "The main loop start";


		Multiplexer m;
		m.MainLoop();
		Logging::Debug() << "The main loop stop";
	}
	catch (const exception &e)
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