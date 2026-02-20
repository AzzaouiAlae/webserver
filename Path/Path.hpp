#pragma once

#include "../Headers.hpp"
#include "../Config/Config.hpp" // Needs access to Server and Location structs
#include <sys/stat.h> // For struct stat

class Path {
    private:
        // --- RESULT DATA ---
        string      _fullPath;      // The absolute path on the disk (e.g., /var/www/html/index.php)
        string      _pathInfo;      // For CGI (e.g., /extra/data)
        string      _cgiScriptPath; // Path to the interpreter or script
		string 		_root;
        
        // --- FLAGS ---
        bool        _isDir;         // Is the resulting path a directory?
        bool        _isFile;        // Is the resulting path a file?
        bool        _isCGI;         // Does it match a CGI extension?
        bool        _found;         // Does the file exist on disk?
		bool        _isRedir;       // <--- NEW FLAG
		bool        _hasPermission;
        string      _redirCode;     // <--- e.g., "301"
        string      _redirPath;     // <--- e.g., "/home"
		Config::Server *srv;

        
        // --- INTERNAL HELPERS ---
        // Checks if a file/dir exists and fills _isDir/_isFile
        void        checkFileExistence(const string &path);
        
        // Joins root + uri correctly, handling slashes
        string      joinPath(const string &root, const string &uri);
		void        checkPermissions(const string &path);

		void _setRootAndCGI(Config::Server &srv);
        void _handleDirectoryIndex(Config::Server &srv);
		void _handleRedirection(Config::Server &srv);
        int         matchedLocationIndex; 

    public:
        Path();
        ~Path();

        // --- MAIN LOGIC ---
        // This replaces the old CreatePath. 
        // It takes the pre-parsed Server config and the Request URI.
        void        CreatePath(Config::Server &srv, const string &reqUrl);

		static std::string decodePath(const std::string& path);
		static std::string encodePath(const std::string& path);


        // --- GETTERS ---
        string      getFullPath() const;
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
        
        // Returns the location used (optional, if you need to know which block matched)
		Config::Server::Location *getLocation();
};
