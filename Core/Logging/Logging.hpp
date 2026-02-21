#pragma once

#include "../Headers.hpp"

#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"

#define	DEBUG	Logging::Debug
#define	INFO	Logging::Info
#define	WARN	Logging::Warn
#define	ERR		Logging::Error

#define DDEBUG(className) Logging::Debug(className, true)

class Logging
{
	ofstream logFile;
	static set<string> activeDebugClasses;
	static set<string> activeDetailClasses;
	static bool globalDebug;
    static bool globalDetail;

	string getTime();
	void WriteDebug(const string &msg);
	void WriteInfo(const string &msg);
	void WriteWarn(const string &msg);
	void WriteError(const string &msg);

public:

	static void EnableDebug(const string &filename);
	static void EnableDetailDebug(const string &filename);
	static Logging *current;
	Logging(string filename);
	~Logging();

	class LogBase
	{
	protected:
		stringstream ss;

	public:
		LogBase();
		virtual ~LogBase();
		template <typename T>
		LogBase &operator<<(const T &value)
		{
			
			ss << value;
			return *this;
		}
	};

	class Debug : public LogBase
	{
		bool isActive;
	public:
		Debug();
		Debug(string ClsName, bool isDetail = false);
		~Debug();
	};

	class Info : public LogBase
	{
	public:
		Info();
		~Info();
	};

	class Warn : public LogBase
	{
	public:
		Warn();
		~Warn();
	};

	class Error : public LogBase
	{
	public:
		Error();
		~Error();
	};
};
