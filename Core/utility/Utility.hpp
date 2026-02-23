#pragma once
#include "../Headers.hpp"

class Utility
{
	static string toTrime;
public:
	static bool SigPipe;
	static bool isNotZero(char ch);
	static bool isNotSquareBracket(char ch);
	static void ltrim(string &s, bool (*f)(char ch));
	static void rtrim(string &s, bool (*f)(char ch));
	static void trim(string &s, bool (*f)(char ch));
	static long CurrentTime();
	static string GetFileExtension(string filename);
	static bool isNotDot(char ch);
	static long getFileSize(const string &path);
	static bool strtosize_t(const string& s, size_t& out);
	static bool isStrToTrime(char ch);
	static void trim(string &s, string toTrime);
	static char HexaToChar(string hex);
	static bool isHexa(string hex);
	static bool isHexa(char hex);
	static void parseBySep(vector<string> &parsedPath, string str, string sep);
	static string lastToken(const string &str, char ch) ;

};
