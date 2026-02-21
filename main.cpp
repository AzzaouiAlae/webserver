#include "Headers.hpp"
#include "Socket/Socket.hpp"
#include "DefaultPages/DefaultPages.hpp"

void sigpipe_handler(int signum)
{
	(void)signum;
	Utility::SigPipe = true;
}

void initDebug(string &className)
{
	if (className.empty())
	{
		INFO() << "Global Debug (-d) enabled.";
		Logging::EnableDebug("");
	}
	else
	{
		INFO() << "Debug (-d) enabled for class: " << className;
		Logging::EnableDebug(className);
	}
}

void initDetailedDebug(string &className)
{
	if (className.empty())
	{
		INFO() << "Global Detailed Debug (-D) enabled";
		Logging::EnableDetailDebug("");
	}
	else
	{
		INFO() << "Detailed Debug (-D) enabled for class: " << className;
		Logging::EnableDetailDebug(className);
	}
}

int ParseLoggingArgs(int argc, char *argv[])
{
	int fileNameIdx = 1;
	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];
		if (arg.length() >= 2 && arg[0] == '-')
		{
			std::string className = arg.substr(2);

			if (arg[1] == 'd')
			{
				initDebug(className);
			}
			else if (arg[1] == 'D')
			{
				initDetailedDebug(className);
			}
			fileNameIdx++;
		}
		else
			break;
	}
	return fileNameIdx;
}

int InitServer(int argc, char *argv[], string &filename)
{
	Logging log("webserver.logs");
	(void)log;
	int fileNameIdx = ParseLoggingArgs(argc, argv);

	signal(SIGPIPE, sigpipe_handler);

	if (argc < fileNameIdx + 1)
		filename = DEFAULT_CONF;
	else
	{
		filename = argv[fileNameIdx];
	}
	Logging::Info() << "Server start with configuation file: " << filename << "";
}

void LaunchServer(string &filename)
{
	DEBUG("main") << "Tokenizing start";
	Tokenizing token(filename);
	token.split_tokens();
	DEBUG("main") << "Tokenizing complete";

	DEBUG("main") << "Parsing start";
	Parsing pars(token.get_tokens());
	pars.BuildAST();
	DEBUG("main") << "Parsing complete";

	DEBUG("main") << "Validation start";
	Validation val(Singleton::GetASTroot());
	val.Validate();
	DEBUG("main") << "Validation complete";

	pars.FillConf();

	DefaultPages::InitDefaultPages();
	DEBUG("main") << "The main loop start";

	Multiplexer m;
	m.MainLoop();
	DEBUG("main") << "The main loop stop";
}

void HandelError(const char *what)
{
	ERR() << what;

	set<AFd *> &fds = Singleton::GetFds();
	set<AFd *>::iterator it = fds.begin();

	for (; it != fds.end(); it = fds.begin())
	{
		delete *it;
	}
}

int main(int argc, char *argv[])
{
	string filename;
	InitServer(argc, argv, filename);
	try
	{
		LaunchServer(filename);
	}
	catch (const exception &e)
	{
		HandelError(e.what());

		return 1;
	}
}
