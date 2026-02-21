#pragma once

#include "../../AbstractSyntaxTree/AST.hpp"
#include <set>

using namespace std;

class DeprecatedConfig
{

public:
	class DeprecatedServer 
	{
	public:
		DeprecatedServer();
		vector<string> listen;
		vector<pair<string, string> > hosts;
		map<string, string> errorPages;
		string serverName;
		vector<string> index;
		string root;
		bool autoindex;
		size_t clientMaxBodySize;
		bool allowMethodExists;
		vector<string> allowMethods;
		bool isMaxBodySize;

		class DeprecatedLocation
		{
		public:
			DeprecatedLocation();
			bool autoindex;
			bool isMaxBodySize;
			size_t clientMaxBodySize;
			bool clientBodyInFileOnly;
			string clientBodyTempPath;
			string cgiPassExt;
			string cgiPassPath;
			string returnCode;
			string returnArg;
			string root;
			vector<string> index;
			string path;
			vector<string> parsedPath;
			bool allowMethodExists;
			vector<string> allowMethods;
			bool deleteFiles;
		};
		vector<DeprecatedLocation> Locations;
	};
	vector<DeprecatedServer> Servers;
	void listenToAllHosts();
	bool IsDuplicatedServer(int currServerIdx, const string& val, const string &srvName) ;
	static DeprecatedConfig::DeprecatedServer &GetServerName(vector<DeprecatedConfig::DeprecatedServer> &srvs, const string &val);
	static string GetErrorPath(DeprecatedConfig::DeprecatedServer &srv, const string &code);
	static int GetLocationIndex(DeprecatedConfig::DeprecatedServer &srv, const string &path);
	static int GetMaxBodySize(vector<DeprecatedConfig::DeprecatedServer> &srvs);
private:
	static bool isPrefixMatch(const vector<string> &locPath, const vector<string> &reqPath);
    static bool pathMatchesCgiExt(const string &path, const string &cgiPassExt);
    static int findBestCgiMatch(DeprecatedConfig::DeprecatedServer &srv, const vector<string> &reqPath, const string &path);
    static int findBestStaticMatch(DeprecatedConfig::DeprecatedServer &srv, const vector<string> &reqPath);
};
#include "../../Headers.hpp"