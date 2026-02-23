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
		AST<string> *srvNode;
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
			AST<string> *locNode;
			Location();
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
		vector<Location> Locations;
	};
	vector<Server> Servers;
	void listenToAllHosts();
	bool IsDuplicatedServer(int currServerIdx, const string& val, const string &srvName) ;
	static Config::Server &GetServerName(vector<Config::Server> &srvs, const string &val);
	static string GetErrorPath(Config::Server &srv, const string &code);
	static int GetLocationIndex(Config::Server &srv, const string &path);
	static int GetMaxBodySize(vector<Config::Server> &srvs);
	static void FillConf();
	static void parseListen(const string &str, string &port, string &host);
private:
	static bool isPrefixMatch(const vector<string> &locPath, const vector<string> &reqPath);
    static bool pathMatchesCgiExt(const string &path, const string &cgiPassExt);
    static int findBestCgiMatch(Config::Server &srv, const vector<string> &reqPath, const string &path);
    static int findBestStaticMatch(Config::Server &srv, const vector<string> &reqPath);
	static void fillServer(AST<string> &serverNode, Config::Server &srv);
	static void fillLocation(AST<string> &locationNode, Config::Server::Location &loc);
	static size_t parseByteSize(const string &raw);
};
#include "../Headers.hpp"