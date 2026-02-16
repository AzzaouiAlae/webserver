#include "Path.hpp"
#include <unistd.h> // for access and F_OK

// Constructor: Initialize all flags to false/empty
Path::Path()
	: 	_isDir(false),
		_isFile(false),
		_isCGI(false),
		_found(false),
		_isRedir(false),
		_hasPermission(false),
		matchedLocationIndex(-1)
{
}

Path::~Path()
{
}

// Helper: Cleanly joins two paths (e.g., "/var/www" + "/index.html")
string Path::joinPath(const string &root, const string &uri)
{
	if (root.empty())
		return uri;
	if (uri.empty())
		return root;

	if (root[root.length() - 1] == '/' && uri[0] == '/')
		return root + uri.substr(1);
	else if (root[root.length() - 1] != '/' && uri[0] != '/')
		return root + "/" + uri;

	return root + uri;
}

// Helper: Checks if path exists and determines if it is a Directory or File
void Path::checkFileExistence(const string &path)
{
	struct stat info;

	// reset defaults
	_found = false;
	_isDir = false;
	_isFile = false;

	if (stat(path.c_str(), &info) == 0)
	{
		_found = true;
		if (info.st_mode & S_IFDIR)
			_isDir = true;
		else if (info.st_mode & S_IFREG)
			_isFile = true;
	}
}

// 2. The Check Function
void Path::checkPermissions(const string &path)
{
    // R_OK checks for Read permission. 
    // For directories, this allows listing files (Autoindex).
    // For files, this allows reading content.
    if (access(path.c_str(), R_OK) == 0) {
        _hasPermission = true;
    } else {
        _hasPermission = false;
    }
}

void Path::_setRootAndCGI(Config::Server &srv)
{
    // Default to Server root
    _root = srv.root; 

    if (matchedLocationIndex != -1) 
    {
        const Config::Server::Location &loc = srv.Locations[matchedLocationIndex];
        
        // Priority: Location Root overrides Server Root
        if (!loc.root.empty()) {
            _root = loc.root;
        }

        // Check for CGI settings
        if (!loc.cgiPassExt.empty()) {
            _isCGI = true;
            _cgiScriptPath = loc.cgiPassPath;
        }
    }
}

void Path::_handleRedirection(Config::Server &srv)
{
    // If no location matched, there can't be a location-based redirection
    if (matchedLocationIndex == -1) return;

    const Config::Server::Location &loc = srv.Locations[matchedLocationIndex];

    // Check if the location block has a "return" directive
    if (!loc.redirectionCode.empty()) 
    {
        _isRedir = true;
        _redirCode = loc.redirectionCode;
        _redirPath = loc.redirectionURI;
    }
}

void Path::_handleDirectoryIndex(Config::Server &srv)
{
    // Get the list of indices (e.g., "index.html", "index.php")
    vector<string> indices = srv.index; // Default to Server indices
    
    // Override with Location indices if available
    if (matchedLocationIndex != -1 && !srv.Locations[matchedLocationIndex].index.empty()) {
        indices = srv.Locations[matchedLocationIndex].index;
    }

    // Try every index file in the list
    for (size_t i = 0; i < indices.size(); ++i) 
    {
        string potentialIndex = joinPath(_fullPath, indices[i]);
        
        struct stat s;
        if (stat(potentialIndex.c_str(), &s) == 0 && (s.st_mode & S_IFREG)) 
        {
            // We found an index file! Update path.
            _fullPath = potentialIndex;
            _found = true;
            _isFile = true;
            _isDir = false;
            
            // IMPORTANT: We must re-check permissions for this specific file
            checkPermissions(_fullPath); 
            break; // Stop after finding the first valid index
        }
    }
}

void Path::CreatePath(Config::Server &srv, const string &reqUrl)
{
    // 1. Find the best matching Location block
    matchedLocationIndex = Config::GetLocationIndex(srv, reqUrl);

    // 2. NEW: Check for Redirection
    _handleRedirection(srv);
    if (_isRedir) {
        return; // STOP HERE! No need to look for files on disk.
    }

    // 3. Set the Root and Check for CGI (Helper 1)
    _setRootAndCGI(srv);

    // 4. Construct the Basic Full Path
    _fullPath = joinPath(_root, reqUrl);
    
    // 5. Check Existence and Permissions
    checkFileExistence(_fullPath);
    if (_found) {
        checkPermissions(_fullPath);
    }

    // 6. Handle Directory Indices if needed (Helper 2)
    if (_isDir && _hasPermission) 
    {
        _handleDirectoryIndex(srv);
    }
}

// 4. Implement Getter

// --- The New Function ---
bool Path::emptyRoot() const
{
    // Returns true ONLY if both Server root AND Location root resulted in an empty string.
    return _root.empty();
}

string Path::getFullPath() const { return _fullPath; }
string Path::getPathInfo() const { return _pathInfo; }
string Path::getCgiPath() const { return _cgiScriptPath; }
bool Path::isCGI() const { return _isCGI; }
bool Path::isDirectory() const { return _isDir; }
bool Path::isFile() const { return _isFile; }
bool Path::isFound() const { return _found; }
bool Path::hasPermission() const { return _hasPermission; }
bool Path::isRedirection() const { return _isRedir; }
string Path::getRedirCode() const { return _redirCode; }
string Path::getRedirPath() const { return _redirPath; }
