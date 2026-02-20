#include "Config.hpp"
#include "../Socket/Socket.hpp"

Config::Server::Server()
{
	autoindex = true;
	clientMaxBodySize = 0;
	allowMethodExists = false;
	isMaxBodySize = false;
}

Config::Server::Location::Location()
{
	allowMethodExists = false;
	clientMaxBodySize = 0;
	isMaxBodySize = false;
}

bool Config::IsDuplicatedServer(int currServerIdx, const string &val, const string &srvName)
{
	for (int i = 0; i < currServerIdx; i++)
	{
		Server srv = Servers[i];
		if (srvName != srv.serverName)
			continue;
		for (int j = 0; j < (int)srv.listen.size(); j++)
		{
			if (val == srv.listen[j])
			{
				return true;
			}
		}
	}
	return false;
}

void Config::listenToAllHosts()
{
	for (int i = 0; i < (int)Servers.size(); i++)
	{
		Server srv = Servers[i];
		for (int j = 0; j < (int)srv.listen.size(); j++)
		{
			if (IsDuplicatedServer(i, srv.listen[j], srv.serverName))
			{
				
				continue;
			}
			Socket::AddSocketNew(srv.hosts[j].first, srv.hosts[j].second, srv);
		}
	}
}

Config::Server &Config::GetServerName(vector<Config::Server> &srvs, const string &val)
{
	for (int i = 0; i < (int)srvs.size(); i++)
	{
		if (val == srvs[i].serverName)
			return srvs[i];
	}
	for (int i = 0; i < (int)srvs.size(); i++)
	{
		vector<string> &hosts = srvs[i].listen;
		for (int j = 0; j < (int)hosts.size(); j++)
		{
			if (val == hosts[j])
				return srvs[i];
		}
	}
	return srvs[0];
}

int Config::GetMaxBodySize(vector<Config::Server> &srvs)
{
	size_t size = 0;
	for (int i = 0; i < (int)srvs.size(); i++)
	{
		if (size < srvs[i].clientMaxBodySize) {
			size = srvs[i].clientMaxBodySize;
		}
		vector < Config::Server::Location> &loc = srvs[i].Locations;
		for(int j = 0; j < (int)loc.size(); j++)
		{
			if (size < loc[j].clientMaxBodySize) {
				size = loc[j].clientMaxBodySize;
			}
		}
	}
	return size;
}

string Config::GetErrorPath(Config::Server &srv, const string &code)
{
	std::map<std::string, std::string>::iterator it = srv.errorPages.find(code);
	if (it != srv.errorPages.end())
	{
		return it->second;
	}
	return "";
}

// Checks if locPath is a prefix of reqPath
bool Config::isPrefixMatch(const vector<string> &locPath, const vector<string> &reqPath)
{
    if (locPath.size() > reqPath.size())
        return false;
    for (size_t j = 0; j < locPath.size(); ++j)
    {
        if (locPath[j] != reqPath[j])
            return false;
    }
    return true;
}

// Checks if the request path ends with the CGI extension
bool Config::pathMatchesCgiExt(const string &path, const string &cgiPassExt)
{
    if (path.length() < cgiPassExt.length())
        return false;
    return path.compare(path.length() - cgiPassExt.length(),
                        cgiPassExt.length(), cgiPassExt) == 0;
}

// Returns the index of the best (longest prefix) CGI location match, or -1
int Config::findBestCgiMatch(Config::Server &srv, const vector<string> &reqPath, const string &path)
{
    int    bestIndex = -1;
    size_t maxLength = 0;

    for (size_t i = 0; i < srv.Locations.size(); ++i)
    {
        const Config::Server::Location &loc = srv.Locations[i];
        if (loc.cgiPassExt.empty())
            continue;
        if (!isPrefixMatch(loc.parsedPath, reqPath))
            continue;
        if (!pathMatchesCgiExt(path, loc.cgiPassExt))
            continue;
        if (loc.parsedPath.size() > maxLength || bestIndex == -1)
        {
            maxLength = loc.parsedPath.size();
            bestIndex = (int)i;
        }
    }
    return bestIndex;
}

// Returns the index of the best (longest prefix) static location match, or -1
int Config::findBestStaticMatch(Config::Server &srv, const vector<string> &reqPath)
{
    int    bestIndex = -1;
    size_t maxLength = 0;

    for (size_t i = 0; i < srv.Locations.size(); ++i)
    {
        const Config::Server::Location &loc = srv.Locations[i];
        if (!loc.cgiPassExt.empty())
            continue;
        if (!isPrefixMatch(loc.parsedPath, reqPath))
            continue;
        if (loc.parsedPath.size() > maxLength || bestIndex == -1)
        {
            maxLength = loc.parsedPath.size();
            bestIndex = (int)i;
        }
    }
    return bestIndex;
}

// Main orchestrator — now just coordinates the helpers
int Config::GetLocationIndex(Config::Server &srv, const string &path)
{
    vector<string> reqPath;
    Utility::parseBySep(reqPath, path, "/");

    int bestCgiIndex = findBestCgiMatch(srv, reqPath, path);
    if (bestCgiIndex != -1)
        return bestCgiIndex;

    return findBestStaticMatch(srv, reqPath);
}
