#include "Headers.hpp"
#include "Socket.hpp"
#include "DefaultPages.hpp"

void sigpipe_handler(int signum)
{
	(void)signum;
	Utility::SigPipe = true;
}

void sigint_handler(int signum)
{
	(void)signum;
	if (Utility::SigInt)
		exit(0);
	Utility::SigInt = true;
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

void SetFileDescriptorLimits()
{
	struct rlimit limit;

	if (getrlimit(RLIMIT_NOFILE, &limit) == 0)
	{
		if (limit.rlim_cur == RLIM_INFINITY)
			Utility::maxFds = 10000;
		else
		{
			Utility::maxFds = limit.rlim_cur / 4;
			if (Utility::maxFds > 10000)
				Utility::maxFds = 10000;
			if (Utility::maxFds < 10 && limit.rlim_cur > 10)
				Utility::maxFds = 5;
		}
	}
}

void CreateDefaultFiles()
{
	Config::createDir();
	Config::writeIndexFile();
	if (access(DEFAULT_CONF, F_OK) == -1)
	{
		ofstream confFile(DEFAULT_CONF);
		if (confFile.is_open())
		{
			confFile
				<< "server {\n"
				<< "}\n"
				<< "types {\n"
				<< "\ttext/html htm html;\n"
				<< "}";
			confFile.close();
			INFO() << "Default configuration file created at: " << DEFAULT_CONF;
		}
		else
			Error::ThrowError("Failed to create default configuration file.");
	}
}

void InitServer(int argc, char *argv[], string &filename)
{
	int fileNameIdx = ParseLoggingArgs(argc, argv);

	signal(SIGPIPE, sigpipe_handler);
	signal(SIGINT, sigint_handler);
	SetFileDescriptorLimits();

	if (argc < fileNameIdx + 1)
	{
		filename = DEFAULT_CONF;
		CreateDefaultFiles();
	}
	else
	{
		filename = argv[fileNameIdx];
	}
	Logging::Info() << "Server start with configuation file: " << filename << "";
}

void LaunchServer(string &filename)
{
	DefaultPages::InitDefaultPages();
	DEBUG("main") << "Tokenizing start";
	Tokenizing token(filename);
	token.split_tokens();
	DEBUG("main") << "Tokenizing complete";

	Logging::Debug() << "Parsing start";
	Parsing pars(token.get_tokens());
	pars.BuildAST();
	Logging::Debug() << "Parsing complete";

	Logging::Debug() << "Validation start";
	Validation val(Singleton::GetASTroot());
	val.Validate();
	Logging::Debug() << "Validation complete";

	Config::FillConf();

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
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
}

int main(int argc, char *argv[])
{
	Logging log("webserver.logs");

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
	catch (...)
	{
		HandelError("Unknown exception occurred during server launch.");
		return 1;
	}
}
