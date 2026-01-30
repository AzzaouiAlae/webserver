#pragma once
#include "../Headers.hpp"
#include <sys/time.h>


class Utility
{
public:
	static bool isNotZero(char ch);
	static bool isNotSquareBracket(char ch);
	static void ltrim(std::string &s, bool (*f)(char ch));
	static void rtrim(std::string &s, bool (*f)(char ch));
	static void trim(std::string &s, bool (*f)(char ch));
	static long CurrentTime();
	static string GetFileExtension(string filename);
	static bool isNotDot(char ch);
	static long getFileSize(const std::string &path);
};
