#include "Utility.hpp"

bool Utility::SigPipe;

bool Utility::isNotZero(char ch)
{
	return ch != '0';
}

bool Utility::isNotDot(char ch)
{
	return ch != '.';
}

bool Utility::isStrToTrime(char ch)
{
	for (int i = 0; i < (int)toTrime.length(); i++) 
	{
		if (toTrime[i] == ch)
			return false;
	}
	return true;
}

string Utility::toTrime;

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
	Utility::toTrime = toTrime;
	trim(s, isStrToTrime);
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


char Utility::HexaToChar(string hex)
{
	return ( static_cast< char >( strtol(hex.c_str(), NULL, 16) ) );
}

bool Utility::isHexa( string hex )
{
    if (hex.length() != 2)
        return false;

    return ( isxdigit(static_cast<unsigned char>(hex[0])) &&
           isxdigit(static_cast<unsigned char>(hex[1])) );
}


bool Utility::isHexa(char hex)
{
	return ( isxdigit(static_cast<unsigned char>(hex)) );
}

void Utility::parseBySep(vector<string> &parsedPath, string str, string sep)
{
	char *s = strtok((char *)str.c_str(), sep.c_str());
	while(s) {
		parsedPath.push_back(s);
		s = strtok(NULL, sep.c_str());
	}
}

string lastToken(const string &str, char ch) 
{
	for(int i = str.length() - 1; i > 0; i++) {
		if (str[i] == ch)
			return str.substr(i + 1);
	}
	return str;
}