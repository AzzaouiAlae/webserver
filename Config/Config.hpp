#pragma once

#include "../AbstractSyntaxTree/AST.hpp"
#include <set>

using namespace std;

class Config
{
	
public:
	class Server 
	{
	public:
		Server();
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

		class Location
		{
		public:
			Location();
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
		};
		vector<Location> Locations;
	};
	vector<Server> Servers;
	void listenToAllHosts();
	bool IsDuplicatedServer(int currServerIdx, const string& val, const string &srvName) ;
	static Config::Server &GetServerName(vector<Config::Server> &srvs, const string &val);
	static string GetErrorPath(Config::Server &srv, const string &code);
	static int GetLocationIndex(Config::Server &srv, const string &path);
	static int GetMaxBodySize(vector<Config::Server> &srvs);
private:
	
};
#include "../Headers.hpp"