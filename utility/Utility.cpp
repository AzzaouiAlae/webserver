#include "Utility.hpp"

bool Utility::isNotZero(char ch)
{
	return ch != '0';
}

bool Utility::isNotDot(char ch)
{
	return ch != '.';
}

bool Utility::isChar(char ch)
{
	return ch != Utility::ch;
}

char Utility::ch;

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

void Utility::trim(std::string &s, string toTrime)
{
	for(int i = 0; (int)toTrime.length() > i; i++)
	{
		Utility::ch = toTrime[i];
		trim(s, isChar);
	}
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


bool Utility::strtosize_t(const std::string& s, size_t& out)
{
    if (s.empty())
        return false;

    size_t result = 0;

    for (size_t i = 0; i < s.size(); i++)
    {
        if (!isdigit(s[i]))
            return false;

        size_t digit = s[i] - '0';

        if (result > (SIZE_MAX - digit) / 10)
            return false;

        result = result * 10 + digit;
    }

    out = result;
    return true;
}

void Utility::Close(int fd)
{
	close(fd);
}