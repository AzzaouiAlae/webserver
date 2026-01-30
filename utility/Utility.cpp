#include "Utility.hpp"

bool Utility::isNotZero(char ch)
{
	return ch != '0';
}

bool Utility::isNotDot(char ch)
{
	return ch != '.';
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

string Utility::GetFileExtension(string filename)
{
	int i;
	for(i = filename.length() - 1; i >= 0; i--)
	{
		if (filename[i] == '.')
			break;
	}
	if (i == -1)
		return "";
	return filename.substr(i + 1);
}

long Utility::getFileSize(const std::string &path)
{
    struct stat st;
    if (stat(path.c_str(), &st) == 0)
        return st.st_size;
    return -1;
}