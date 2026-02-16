#include "Config.hpp"
#include "../Socket/Socket.hpp"

Config::Server::Server()
{
	autoindex = true;
	clientMaxBodySize = -1;
	allowMethodExists = false;
}

Config::Server::Location::Location()
{
	allowMethodExists = false;
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
				Logging::Warn() << "server name: " << srv.serverName << ", Host: " << srv.listen[j] << ", Are duplicated";
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

string Config::GetErrorPath(Config::Server &srv, const string &code)
{
	std::map<std::string, std::string>::iterator it = srv.errorPages.find(code);
	if (it != srv.errorPages.end())
	{
		return it->second;
	}
	return "";
}

int Config::GetLocationIndex(Config::Server &srv, const string &path)
{
	vector<string> reqPath;
	Utility::parseBySep(reqPath, path, "/");

	int bestCgiIndex = -1;
	size_t maxCgiLength = 0;

	int bestStaticIndex = -1;
	size_t maxStaticLength = 0;

	for (size_t i = 0; i < srv.Locations.size(); ++i)
	{
		const Config::Server::Location &loc = srv.Locations[i];
		const vector<string> &locPath = loc.parsedPath;

		// 1. Basic Prefix Check
		// If location path is longer than request path, it can't be a match.
		if (locPath.size() > reqPath.size())
			continue;

		bool isPrefixMatch = true;
		for (size_t j = 0; j < locPath.size(); ++j)
		{
			if (locPath[j] != reqPath[j])
			{
				isPrefixMatch = false;
				break;
			}
		}

		if (!isPrefixMatch)
			continue;

		// 2. Logic Split: Is this a CGI Location or a Standard Location?
		if (!loc.cgiPassExt.empty())
		{
			// --- CGI LOCATION Logic ---
			// It only counts as a match if the Request Extension matches the Location's Extension.
			if (path.length() >= loc.cgiPassExt.length())
			{
				// Check if path ends with cgiPassExt (e.g., ".php")
				if (path.compare(path.length() - loc.cgiPassExt.length(),
								loc.cgiPassExt.length(), loc.cgiPassExt) == 0)
				{
					// It is a valid CGI match. Check if it is the longest/most specific so far.
					// Note: We use >= to allow overriding earlier matches of same length (optional)
					if (locPath.size() > maxCgiLength || bestCgiIndex == -1)
					{
						maxCgiLength = locPath.size();
						bestCgiIndex = (int)i;
					}
				}
			}
		}
		else
		{
			// --- STATIC LOCATION Logic ---
			// Standard longest prefix match
			if (locPath.size() > maxStaticLength || bestStaticIndex == -1)
			{
				maxStaticLength = locPath.size();
				bestStaticIndex = (int)i;
			}
		}
	}

	// 3. Final Decision: Priority to CGI
	if (bestCgiIndex != -1)
	{
		return bestCgiIndex;
	}
	return bestStaticIndex;
}
