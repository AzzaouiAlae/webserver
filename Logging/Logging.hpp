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

class Logging
{
	ofstream logFile;
	void Write(int level, const std::string &msg);

public:
	static Logging *current;
	Logging(string filename);
	~Logging();

	class LogBase
	{
	protected:
		std::stringstream ss;
		int level;

	public:
		LogBase(int lvl);
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
	public:
		Debug();
	};

	class Info : public LogBase
	{
	public:
		Info();
	};

	class Warn : public LogBase
	{
	public:
		Warn();
	};

	class Error : public LogBase
	{
	public:
		Error();
	};
};
