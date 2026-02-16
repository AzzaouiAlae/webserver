#pragma once
#include "../Headers.hpp"

class Config
{
public:
	class Server 
	{
	public:
		Server();
		vector<pair<string, string> > listen;
		map<string, string> errors;
		string serverName;
		string index;
		string root;
		bool autoindex;
		int clientMaxBodySize;
		map<string, bool> methods;

		class Location
		{
		public:
			Location();
			pair<string, string> cgi_pass;
			pair<string, string> redirection;
			string root;
			string index;
			string path;
			map<string, bool> methods;
		};
		vector<Location> Locations;
	};
	vector<Server> Servers;
};
