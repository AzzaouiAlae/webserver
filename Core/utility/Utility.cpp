#include "Utility.hpp"
#include "HTTPContext.hpp"

bool Utility::SigPipe;
bool Utility::SigInt;
vector<char *> Utility::buffPoll;
long Utility::maxFds;

void Utility::clearFds()
{
	set<AFd *> &fds = Singleton::GetFds();
	while (!fds.empty())
	{
		AFd *obj = *fds.begin();
		fds.erase(fds.begin());
		delete obj;
	}
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

void Utility::ltrim(string &s,  string toTrime)
{
	Utility::toTrime = toTrime;
	s.erase(s.begin(), find_if(s.begin(), s.end(), isStrToTrime));
}

void Utility::rtrim(string &s, string toTrime)
{
	Utility::toTrime = toTrime;

	string::reverse_iterator reverse_it = find_if(s.rbegin(), s.rend(), isStrToTrime);
	string::iterator *it = (string::iterator *)&reverse_it;
	s.erase(*it, s.end());
}

void Utility::trim(string &s, string toTrime)
{
	rtrim(s, toTrime);
	ltrim(s, toTrime);
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
	for (i = filename.length() - 1; i >= 0; i--)
	{
		if (filename[i] == '.')
			break;
	}
	if (i == -1)
		return "";
	return filename.substr(i + 1);
}

long Utility::getFileSize(const string &path)
{
	struct stat st;
	if (stat(path.c_str(), &st) == 0)
		return st.st_size;
	return -1;
}

bool Utility::strtosize_t(const string &s, size_t &out)
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
	return (static_cast<char>(strtol(hex.c_str(), NULL, 16)));
}

bool Utility::isHexa(string hex)
{
	if (hex.length() != 2)
		return false;

	return (isxdigit(static_cast<unsigned char>(hex[0])) &&
			isxdigit(static_cast<unsigned char>(hex[1])));
}

bool Utility::isHexa(char hex)
{
	return (isxdigit(static_cast<unsigned char>(hex)));
}

void Utility::parseBySep(vector<string> &parsedPath, string str, string sep)
{
	char *s = strtok((char *)str.c_str(), sep.c_str());
	while (s)
	{
		parsedPath.push_back(s);
		s = strtok(NULL, sep.c_str());
	}
}

string Utility::lastToken(const string &str, char ch)
{
	for (int i = str.length() - 1; i > 0; i++)
	{
		if (str[i] == ch)
			return str.substr(i + 1);
	}
	return str;
}

size_t Utility::parseByteSize(const string &raw)
{
	char *endptr = NULL;
	size_t value = (size_t)strtoll(raw.c_str(), &endptr, 10);

	if (endptr && *endptr != '\0')
	{
		char unit = *endptr;
		if (unit == 'k' || unit == 'K')
			value *= 1024;
		else if (unit == 'm' || unit == 'M')
			value *= 1024 * 1024;
		else if (unit == 'g' || unit == 'G')
			value *= 1024 * 1024 * 1024;
	}

	return value;
}

char *Utility::GetBuffer()
{
	if (buffPoll.size() > 0)
	{
		char *buffer = buffPoll.back();
		buffPoll.pop_back();
		return buffer;
	}
	return new char[BUF_SIZE + 1];	
}

void Utility::LogBufferPullSize()
{
	Logging::Info() << "Current buffer pool size: " << buffPoll.size();
}

void Utility::ReleaseBuffer(char *buffer)
{
	if (buffer && buffPoll.size() < 200) {
		buffPoll.push_back(buffer);
	} 
	else
		delete[] buffer;
}

void Utility::ClearBuffPoll()
{
	for (size_t i = 0; i < buffPoll.size(); i++)
		delete[] buffPoll[i];
	buffPoll.clear();
}

size_t Utility::GetBuffPollSize()
{
	return buffPoll.size();
}

void Utility::normalizePath(string &path)
{
	if (path.empty())
		return;
	bool isAbsolute = path[0] == '/';
	bool hasTrailingSlash = path[path.length() - 1] == '/';
	vector<string> segments;
	parseBySep(segments, path, "/");
	vector<string> normalizedSegments;

	for (size_t i = 0; i < segments.size(); i++)
	{
		if (segments[i] == "" || segments[i] == ".")
			continue;
		else if (segments[i] == "..")
		{
			if (!normalizedSegments.empty())
				normalizedSegments.pop_back();
		}
		else
			normalizedSegments.push_back(segments[i]);
	}

	string normalizedPath = isAbsolute ? "/" : "";
	for (size_t i = 0; i < normalizedSegments.size(); i++)
	{
		normalizedPath += normalizedSegments[i];
		if (i != normalizedSegments.size() - 1)
		{
			normalizedPath += "/";
		}
	}
	if (hasTrailingSlash && (int)normalizedPath.size() > 1)
	{
		normalizedPath += "/";
	}
	path = normalizedPath;
}

bool Utility::getAbsolute(string &path)
{
	if (path.empty())
		return false;

	if (path[0] != '/')
	{
		char cwd[PATH_MAX];

		if (getcwd(cwd, sizeof(cwd)) != NULL)
		{
			string currentDir(cwd);
			if (!currentDir.empty() && currentDir[currentDir.length() - 1] != '/')
				currentDir += '/';
			path = currentDir + path;
			return true;
		}
		else
			ERR() << "Error getting current working directory";
	}
	return false;
}

string Utility::getRandomStr(size_t length)
{
	stringstream ss;
	ss << hex;
	for (size_t i = 0; i < length; i++)
		ss << (rand() % 16);
	return ss.str();
}

string Utility::addRandomStr(string filename)
{
	if (filename.empty())
		return Utility::getRandomStr();
	
	string randName = Utility::getRandomStr(6);
	size_t lastDot = filename.find_last_of(".");
	size_t lastSlash = filename.find_last_of("/");
	if (lastDot == string::npos || (lastDot < lastSlash && lastSlash != string::npos))
		return filename + "_" + randName;
	return filename.substr(0, lastDot) + "_" + randName + filename.substr(lastDot);
}
