#include "Utility.hpp"

bool Utility::isNotZero(char ch)
{
	return ch != '0';
}

bool Utility::isNotSquareBracket(char ch)
{
	return ch != ']' && ch != '[';
}

void Utility::ltrim(std::string &s, bool (*f)(char ch))
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), f));
}

void Utility::rtrim(std::string &s, bool (*f)(char ch))
{
	string::reverse_iterator reverse_it = std::find_if(s.rbegin(), s.rend(), f);
	string::iterator *it = (string::iterator *)&reverse_it;
	s.erase(*it, s.end());
}

void Utility::trim(std::string &s, bool (*f)(char ch))
{
	ltrim(s, f);
	rtrim(s, f);
}

long Utility::CurrentTime()
{
	timeval time;
	gettimeofday(&time, NULL);
	return time.tv_sec * USEC + time.tv_usec;
}
