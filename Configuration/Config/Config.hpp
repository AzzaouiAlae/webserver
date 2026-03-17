#pragma once

#include "AST.hpp"
#include <set>

using namespace std;

class Config
{

public:
	class Server 
	{
	public:
		Server();
		AST<string> *srvNode;
		vector<string> listen;
		vector<pair<string, string> > hosts;
		map<string, string> errorPages;
		string serverName;
		vector<string> index;
		string root;
		int autoindex;
		size_t clientMaxBodySize;
		bool allowMethodExists;
		vector<string> allowMethods;
		bool isMaxBodySize;
		int keepAliveTimeout;
		int clientReadTimeout;
		int cgiTimeout;

		class Location
		{
		public:
			AST<string> *locNode;
			Location();
			int autoindex;
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
			bool chunkedSend;
		};
		vector<Location> Locations;
	};
	vector<Server> Servers;
	void listenToAllHosts();
	bool IsDuplicatedServer(int currServerIdx, const string& val, const string &srvName) ;
	static Config::Server &GetServerName(vector<Config::Server> &srvs, const string &val);
	static string GetErrorPath(Config::Server &srv, const string &code);
	static int GetLocationIndex(Config::Server &srv, const string &path, string &scriptPath, string &pathInfo, const string &method);
	static size_t GetMaxBodySize(vector<Config::Server> &srvs);
	static int GetMaxClientReadTimeout(vector<Config::Server> &srvs);
	static void FillConf();
	static void parseListen(const string &str, string &port, string &host);
	static void writeIndexFile();
	static void createDir();
private:
	static bool _foundMethod;
	static bool isPrefixMatch(const vector<string> &locPath, const vector<string> &reqPath);
	
    static int findBestCgiMatch(Config::Server &srv, const vector<string> &reqPath, string &scriptPath, string &pathInfo, const string &method);
    static int findBestStaticMatch(Config::Server &srv, const vector<string> &reqPath);
	static void fillServer(AST<string> &serverNode, Config::Server &srv);
	static void fillLocation(AST<string> &locationNode, Config::Server::Location &loc);
	static size_t parseByteSize(const string &raw);
	static bool isAllowedMethod(const vector<string> &allowMethods, const string &method);
};
#include "Headers.hpp"