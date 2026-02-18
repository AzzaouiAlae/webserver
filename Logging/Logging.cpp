#include "Logging.hpp"

Logging *Logging::current = NULL;

Logging::Logging(string filename)
{
	current = this;
	logFile.open(filename.c_str(), ios::app);
}

Logging::~Logging()
{
	if (logFile.is_open())
	{
		logFile.close();
	}
}

Logging::LogBase::LogBase(int lvl) : level(lvl)
{
}

Logging::Debug::Debug() : LogBase(0)
{
}

Logging::Info::Info() : LogBase(1)
{
}

Logging::Warn::Warn() : LogBase(2)
{
}

Logging::Error::Error() : LogBase(3)
{
}

Logging::LogBase::~LogBase()
{
	if (Logging::current)
	{
		Logging::current->Write(level, ss.str());
	}
}

void Logging::Write(int level, const string &msg)
{
	time_t now = time(0);
	tm *ltm = localtime(&now);
	char buffer[80];
	strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", ltm);
	string timeStr(buffer);
	string prefix, color;
	switch (level)
	{
	case 0:
		prefix = "[DEBUG] ";
		color = BLUE;
		break;
	case 1:
		prefix = "[INFO]  ";
		color = GREEN;
		break;
	case 2:
		prefix = "[WARN]  ";
		color = YELLOW;
		break;
	case 3:
		prefix = "[ERROR] ";
		color = RED;
		break;
	}

	string finalMsg = prefix + timeStr + " : " + msg;

	if (level != 3)
		cout << color << finalMsg << RESET << endl;
	else
		cerr << color << finalMsg << RESET << endl;
	if (logFile.is_open())
	{
		logFile << finalMsg << endl;
		logFile.flush();
	}
}
