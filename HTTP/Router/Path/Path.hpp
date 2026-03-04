#pragma once

#include "Headers.hpp"

class Path {
    private:
        string      _fullPath;
        string      _pathInfo;
        string      _cgiScriptPath;
		string 		_root;

        bool        _isDir;
        bool        _isFile;
        bool        _isCGI;
        bool        _found;
		bool        _isRedir;
		bool        _hasPermission;
        string      _redirCode;
        string      _redirPath;
		string		_decodedPath;
		Config::Server *srv;

        void        checkFileExistence(const string &path);

        string      joinPath(const string &root, const string &uri);
		void        checkPermissions(const string &path);

		void _setRootAndCGI(Config::Server &srv);
        void _handleDirectoryIndex(Config::Server &srv);
		void _handleRedirection(Config::Server &srv);
        int         matchedLocationIndex; 

    public:
        Path();
        ~Path();

        void        CreatePath(Config::Server &srv, const string &reqUrl);

		static string decodePath(const string& path);
		static string encodePath(const string& path);

        string      &getFullPath();
        string      getPathInfo() const;
        string      getCgiPath() const;
        bool        isCGI() const;
        bool        isDirectory() const;
        bool        isFile() const;
        bool        isFound() const;
		bool 		emptyRoot() const;
		bool        hasPermission() const;
		bool        isRedirection() const;
		string      getRedirCode() const;
        string      getRedirPath() const;

		Config::Server::Location *getLocation();
};
