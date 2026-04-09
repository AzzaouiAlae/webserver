#pragma once

#include "Headers.hpp"

struct originalPath {
    string path;
    bool isDir;
    bool isFile;
    bool found;
};

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
        bool        _isFolderRedir;
		bool        _hasPermission;
        string      _redirCode;
        string      _redirPath;
		string		_decodedPath;
        string      _method;
		Config::Server *srv;
        int         matchedLocationIndex; 
        Config::Server::Location *_originalLoc;

        void        checkFileExistence(const string &path);

        string      joinPath(const string &root, const string &uri);
		void        checkPermissions(const string &path);

		void _setRootAndCGI(Config::Server &srv);
        void _handleDirectoryIndex(Config::Server &srv);
		void _handleRedirection(Config::Server &srv);
        
        void _handleFolderRedirection();
    public:
        Path();
        ~Path();
        originalPath OriginalPath;
        void        CreatePath(Config::Server &srv, const string &reqUrl, const string &method);

		static string decodePath(const string& path);
		static string encodePath(const string& path);

        string      &getFullPath();
        string      getPathInfo() const;
        string      getCgiPath() const;
        bool        isCGI() const;
        bool        isDirectory() const;
        bool        isRedirectionToDir() const;
        bool        isFile() const;
        bool        isFound() const;
		bool        hasPermission() const;
		bool        isRedirection() const;
		string      getRedirCode() const;
        string      getRedirPath() const;
        string getDecodePath();
        Config::Server *getServer();
        string &getScriptPath();
        string &getPathInfo();
		Config::Server::Location *getLocation();
        string getMethod() const { return _method; }
        Config::Server::Location *getOriginalLocation();
        void setOriginalLocation(Config::Server::Location *loc);
};
