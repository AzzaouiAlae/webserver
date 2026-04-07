#pragma once
#include "Headers.hpp"
 
#define BUF_SIZE 1024 * 1024

class Utility
{
	static string toTrime;
	static vector<char *> buffPoll;
public:
	static long maxFds;
	static size_t GetBuffPollSize();
	static char *GetBuffer();
	static void ReleaseBuffer(char *buffer);
	static void LogBufferPullSize();
	static void ClearBuffPoll();
	static bool SigPipe;
	static bool SigInt;
	static void ltrim(string &s, string toTrime);
	static void rtrim(string &s, string toTrime);
	static long CurrentTime();
	static string GetFileExtension(string filename);
	static long getFileSize(const string &path);
	static bool strtosize_t(const string& s, size_t& out);
	static bool isStrToTrime(char ch);
	static void trim(string &s, string toTrime);
	static char HexaToChar(string hex);
	static bool isHexa(string hex);
	static bool isHexa(char hex);
	static void parseBySep(vector<string> &parsedPath, string str, string sep);
	static string lastToken(const string &str, char ch) ;
	static size_t parseByteSize(const string &raw);
	static void normalizePath(string &path);
	static bool getAbsolute(string &path);
	static string getRandomStr(size_t length = 16);
	static string addRandomStr(string filename);
	static void clearFds();
};
