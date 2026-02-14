#pragma once
#include "../Headers.hpp"

class Utility
{
	static string toTrime;
public:
	static bool SigPipe;
	static bool isNotZero(char ch);
	static bool isNotSquareBracket(char ch);
	static void ltrim(std::string &s, bool (*f)(char ch));
	static void rtrim(std::string &s, bool (*f)(char ch));
	static void trim(std::string &s, bool (*f)(char ch));
	static long CurrentTime();
	static string GetFileExtension(string filename);
	static bool isNotDot(char ch);
	static long getFileSize(const std::string &path);
	static bool strtosize_t(const std::string& s, size_t& out);
	static bool isStrToTrime(char ch);
	static void trim(std::string &s, string toTrime);
	static void Close(int fd);
	static char HexaToChar(string hex);
};
