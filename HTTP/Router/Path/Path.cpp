#include "Path.hpp"
#include <unistd.h>

Path::Path() : 	_isDir(false), _isFile(false), _isCGI(false),
                _found(false), _isRedir(false), _hasPermission(false),
                srv(NULL), matchedLocationIndex(-1)
{}

Path::~Path() {}

Config::Server::Location *Path::getLocation()
{
	if (matchedLocationIndex == -1 || srv == NULL || 
		(int)srv->Locations.size() <= matchedLocationIndex)
	{
		DDEBUG("Path") << "getLocation: no matched location (index=" << matchedLocationIndex << ")";
		return NULL;
	}
	DDEBUG("Path") << "getLocation: matched location index=" << matchedLocationIndex
					<< ", path='" << srv->Locations[matchedLocationIndex].path << "'";
	return &(srv->Locations[matchedLocationIndex]);
}

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

void Path::checkFileExistence(const string &path)
{
	struct stat info;

	if (stat(path.c_str(), &info) == 0)
	{
		_found = true;
		if (info.st_mode & S_IFDIR)
			_isDir = true;
		else if (info.st_mode & S_IFREG)
			_isFile = true;
	}
	DDEBUG("Path") << "checkFileExistence('" << path << "'): found=" << _found
					<< ", isDir=" << _isDir << ", isFile=" << _isFile;
}

void Path::checkPermissions(const string &path)
{
    if (access(path.c_str(), R_OK) == 0) {
        _hasPermission = true;
    } else {
        _hasPermission = false;
    }
    DDEBUG("Path") << "checkPermissions('" << path << "'): hasPermission=" << _hasPermission;
}

void Path::_setRootAndCGI(Config::Server &srv)
{
    _root = srv.root; 

    if (matchedLocationIndex != -1) 
    {
        const Config::Server::Location &loc = srv.Locations[matchedLocationIndex];
        if (!loc.root.empty()) {
            _root = loc.root;
            DDEBUG("Path") << "_setRootAndCGI: using location root '" << _root << "'";
        }

        if (!loc.cgiPassExt.empty()) {
            _isCGI = true;
            _cgiScriptPath = loc.cgiPassPath;
            DDEBUG("Path") << "_setRootAndCGI: CGI detected, ext='" << loc.cgiPassExt << "', script='" << _cgiScriptPath << "'";
        }
    }
    DDEBUG("Path") << "_setRootAndCGI: root='" << _root << "', isCGI=" << _isCGI;
}

void Path::_handleRedirection(Config::Server &srv)
{
    if (matchedLocationIndex == -1) return;

    const Config::Server::Location &loc = srv.Locations[matchedLocationIndex];
    if (!loc.returnCode.empty()) 
    {
        _isRedir = true;
        _redirCode = loc.returnCode;
        _redirPath = loc.returnArg;
        DDEBUG("Path") << "_handleRedirection: code=" << _redirCode << ", target='" << _redirPath << "'";
    }
}

void Path::_handleDirectoryIndex(Config::Server &srv)
{
    vector<string> indices = srv.index;

    if (matchedLocationIndex != -1 ) {
		if (_decodedPath != srv.Locations[matchedLocationIndex].path)
			return;
        indices = srv.Locations[matchedLocationIndex].index;
    }
	else if (_decodedPath != "/")
		return;

    DDEBUG("Path") << "_handleDirectoryIndex: trying " << indices.size() << " index file(s) in '" << _fullPath << "'";

    for (size_t i = 0; i < indices.size(); ++i) 
    {
        string potentialIndex = joinPath(_fullPath, indices[i]);
        DDEBUG("Path") << "  -> trying index[" << i << "]: '" << potentialIndex << "'";
        
        struct stat s;
        if (stat(potentialIndex.c_str(), &s) == 0 && (s.st_mode & S_IFREG)) 
        {
            _fullPath = potentialIndex;
            _found = true;
            _isFile = true;
            _isDir = false;

            checkPermissions(_fullPath); 
			if (!_hasPermission) 
				continue;
            DDEBUG("Path") << "  -> found index file: '" << _fullPath << "'";
            break;
        }
    }
}

void Path::CreatePath(Config::Server &srv, const string &reqUrl)
{
	_decodedPath = decodePath(reqUrl);
	DEBUG("Path") << "Creating path for URL: '" << reqUrl << "', decoded: '" << _decodedPath << "'";

    matchedLocationIndex = Config::GetLocationIndex(srv, _decodedPath);
	DEBUG("Path") << "Best matching location index: " << matchedLocationIndex;
	DDEBUG("Path") << "  -> Server root='" << srv.root << "', locations=" << srv.Locations.size();
	this->srv = &srv;

    _handleRedirection(srv);
    if (_isRedir) {
		DEBUG("Path") << "Redirection detected: " << _redirCode << " -> " << _redirPath;
        return;
    }

    _setRootAndCGI(srv);

    _fullPath = joinPath(_root, _decodedPath);
    DDEBUG("Path") << "  -> Constructed full path: '" << _fullPath << "'";

    checkFileExistence(_fullPath);
    if (_found) {
        checkPermissions(_fullPath);
    }

    if (_isDir && _hasPermission) {
        _handleDirectoryIndex(srv);
    }
	DEBUG("Path") << "Resolved full path: '" << _fullPath << "' [found=" << _found
				  << ", isDir=" << _isDir << ", isFile=" << _isFile
				  << ", isCGI=" << _isCGI << ", hasPermission=" << _hasPermission << "]";
}

bool Path::emptyRoot() const{ return _root.empty(); }

string Path::encodePath(const string& path)
{
    ostringstream escaped;
    escaped.fill('0');
    escaped << hex << uppercase;

    for (size_t i = 0; i < path.length(); ++i) {
        unsigned char c = path[i];

        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~' || c == '/') {
            escaped << c;
        } else {
            escaped << '%' << setw(2) << static_cast<int>(c);
        }
    }

    return escaped.str();
}

string Path::decodePath(const string& path) 
{
    string decoded;
    decoded.reserve(path.length());

    for (size_t i = 0; i < path.length(); ++i) {
        if (path[i] == '%' && i + 2 < path.length() && 
            isxdigit(path[i + 1]) && isxdigit(path[i + 2])) {

            string hexStr = path.substr(i + 1, 2);

            char decodedChar = static_cast<char>(strtol(hexStr.c_str(), NULL, 16));
            decoded += decodedChar;
            i += 2;
        }
        else {
            decoded += path[i];
        }
    }

    return decoded;
}

string &Path::getFullPath() { return _fullPath; }
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
