#include "Path.hpp"

Path::Path() : _isCGI(false), _isLocationCGI(false)
{}

void    Path::initData(AST<string> *node, string path)
{
	_srvNode = node;
	_requestPath = path;
	_srvRootPath = SearchInTree((*_srvNode), "root")[0];
    _srvIndex = SearchInTree((*_srvNode), "index")[0];
}

vector<string> Path::SearchInTree(AST<string>& node, string value )
{
	vector<AST<string> >& ch = node.GetChildren();
	for (int pos = 0; pos < (int)ch.size(); pos++)
	{
		if (ch[pos].GetValue() == value)
		{
			return (ch[pos].GetArguments());
		}
	}
	return vector<string>(1, "");
}

string     Path::AttachPath(string rootPath, string addPath)
{
    if (rootPath.empty() && addPath.empty())
        return "";

    int Back = rootPath.size() - 1;

    if (rootPath[Back] == '/' && addPath[0] == '/')
        return rootPath + addPath.substr(1);
    else if (rootPath[Back] != '/' && addPath[0] != '/')
        return rootPath + "/" + addPath;
    else
        return rootPath + addPath;
}

bool Path::IsIndexPath(string requestPath, string locArgPath)
{
    cout << "{" << requestPath << "     " << locArgPath << "}\n";
    if ( requestPath == locArgPath )
        return ( true );
    return ( false );
}
string Path::findRootPath(AST<string>& currNode)
{
    _locaRootPath = SearchInTree( currNode, "root")[0];
    
    if ( _locaRootPath.empty() )
         return ( _srvRootPath );
    
    return ( _locaRootPath );
}
string     Path::FullPath( AST<string>& currNode )
{
    string rootPath, locationPath;
    rootPath = findRootPath(currNode);
    if ( IsIndexPath( _requestPath, _locaArgPath ) == true )
        locationPath = AttachIndex( currNode, _requestPath, "location" );
    else
        locationPath = _requestPath;
    _FullPath = AttachPath(rootPath, locationPath) ;
    cout << "<" << rootPath << "        " << locationPath << ">\n";
    return _FullPath;
}


string    Path::AttachIndex( AST<string>& currLocationNode, string path, string type )
{
    string pathWithIndex;
    _locaIndex = SearchInTree(currLocationNode, "index")[0];

    if ( type == "server" )
        pathWithIndex = AttachPath(path, _srvIndex);
    else if ( type == "location" )
        pathWithIndex = AttachPath(path, _locaIndex);
    return ( pathWithIndex );
}

void    DecodeString(string& str)
{
    (void)str;
}

void    Path::CkeckPath(vector<string>& strs)
{
    if ( strs.empty() || strs.size() == 1 )
        return ;

    for (int i = 0; i < (int)strs.size(); i++)
    {
        if ( strs[i] == ".." )
        {
            strs.erase(strs.begin() + i);
            if ( i > 0)
                strs.erase(strs.begin() + (i - 1));
            i = -1;
        }
        if ( strs[i] == "." )
            strs.erase(strs.begin() + i);
        if ( strs[i].find('%') != string::npos )
            DecodeString(strs[i]);
    }
}

void Path::parsePath(vector<string>& strs, string& str,const string& sep)
{

    char *s = strtok((char *)str.c_str(), sep.c_str());
    if (s == NULL)
        return;
    strs.push_back(s);
    while (true)
    {
        s = strtok(NULL, sep.c_str());
        if (s == NULL)
            break;
        strs.push_back(s);
    }
    CkeckPath(strs);

}

int Path::vectorCmp( vector<string>& reqPath, vector<string>&  locationPath )
{
    if (locationPath.size() > reqPath.size())
        return 0;
    if ( locationPath.size() == 0 )
        return 1;
    int i;
    for(i = 0; i < (int)reqPath.size(); i++)
    {
        if (reqPath[i] != locationPath[i])
        {
            if (i < (int)locationPath.size() )
                return 0;
            return i + 1;
        }
    }
    return i + 1;
}

void    Path::append_with_sep(string& result, vector<string>& vec, string sep, int pos )
{
    if (vec.empty() )
        return ;

    if ( pos > (int)vec.size() )
        return ;
    result += vec[pos];
    pos++;
    for ( ; pos < (int)vec.size(); pos++)
        result += (sep + vec[pos]);
}

void        Path::fillLocationInfo( AST<string> & locaNode )
{
    FullPath( locaNode);
    _requestPathNode = &locaNode;
}

string  Path::getErrorPagePath(AST<string> & srvNode, string srvPath, string errorCode)
{
    vector<AST<string> >& ch = srvNode.GetChildren();

    for (int pos = 0; pos < (int)ch.size(); pos++)
	{
	    if (ch[pos].GetValue() == "error_page")
	    {
	    	vector<string> errorpages = ch[pos].GetArguments();
            if ( !errorpages.empty() && find(errorpages.begin(), errorpages.end(), errorCode) != errorpages.end() )
                return ( AttachPath(srvPath, errorpages[errorpages.size() - 1]) );
	    }
    }

    return ( "./" + errorCode + ".html");
}

void    Path::IsDirectory(struct stat info, string& path)
{
    if (S_ISDIR(info.st_mode))
    {
        if (!(info.st_mode & S_IRUSR) || !(info.st_mode & S_IXUSR))
        {
            path = getErrorPagePath((*_srvNode), _srvRootPath, "403");
            _pathType = Error;
            return ;
        }
        _pathType = Dir;
    }
}

void    Path::IsFile(struct stat info, string& path)
{
    if ( S_ISREG(info.st_mode) )
    {
        int fd = open(path.c_str(), O_RDONLY);
        if (fd < 0)
        {
            path = getErrorPagePath((*_srvNode), _srvRootPath, "403");
            _pathType = Error;
            return ;
        }
        close(fd);
        _pathType = File;
    }
}

void    Path::CheckPathExist(string& path)
{
    struct stat info;
    if ( stat(path.c_str(), &info) != 0 )
    {
        Error::errorType = NotFound;
        path = getErrorPagePath((*_srvNode), _srvRootPath, "404");
    }
    IsDirectory(info, path);
    IsFile(info, path);
}

void    Path::HandleRequestPath(vector<string>& vecReqPath)
{
    parsePath(vecReqPath, _requestPath, "/");
    _requestPath.clear();
    append_with_sep(_requestPath, vecReqPath, "/");
}

void    Path::HandleLocationArgPath( vector<string>& vLocaArgPath, string locationArg )
{
    vLocaArgPath.clear();
    _locaArgPath = locationArg;
    if ( _locaArgPath[_locaArgPath.size() - 1] != '/' )
        _locaArgPath += "/";
    parsePath(vLocaArgPath, _locaArgPath, "/");
    _locaArgPath.clear();
    append_with_sep(_locaArgPath, vLocaArgPath, "/");
}

void    Path::HandleSRVPath()
{
    cout << "'" << _requestPath << "'\n";
    if ( _requestPath.empty() )
        _FullPath = AttachPath(_srvRootPath, _srvIndex);
    _FullPath = AttachPath(_srvRootPath, _requestPath);
    _requestPathNode = &(*_srvNode);
}

bool    Path::IsSuranded(string str, char begin, char end)
{
    if ( str[0] == begin && str[str.size() - 1] == end )
        return ( true );
    return ( false );
}

string Path::getRequestExtantion(string path, string p)
{
	string::size_type point = path.find_last_of(p);
    if  (point == string::npos || ( point + 1 ) >= path.size() )
		return ( "" );
	if ( path[point + 1] == '/' )
        return "";
    string::size_type slash = path.find('/', point);
    if ( slash == string::npos)
        slash = path.size();


    return ( path.substr(point, slash - point ) );
}

string getLocationExtantion(string path)
{
	string::size_type start = path.find_last_of('<');
	string::size_type end = path.find_last_of('>');
	string::size_type point = path.find_last_of('.');
	string::size_type notFound = string::npos;
	if (start == notFound || end == notFound || point == notFound )
		return ( "" );
	if ( start > end || point != (start + 1) || point > end )
		return ( "" );
	return ( path.substr(point, end - point) );
}

bool Path::checkCGI(string first, string second, string& Extantion)
{
    string reqExt = getRequestExtantion(first, ".");
    string locaExt = getLocationExtantion(second);
    if ( reqExt.empty() || locaExt.empty() )
        return ( false );
    if ( reqExt != locaExt )
        return ( false );
    Extantion = reqExt;
    return ( true );
}

bool Path::IsCGI(string str)
{
    string reqExt = getRequestExtantion(str, ".");
    if ( reqExt.empty() )
        return ( false );
    _reqExt = reqExt;
    return ( true );
}

void    Path::HandleCGI(AST<string> & locaNode, vector<string>& vreqPath)
{
    string extantion, path;
    _requestPathNode = &locaNode;
    path = findRootPath(locaNode);
    for (size_t i = 0; i < vreqPath.size() ; i++)
    {
        extantion = getRequestExtantion(vreqPath[i], ".");
        if (extantion.empty() )
            path = AttachPath(path, vreqPath[i]);
        else {
            path = AttachPath(path, vreqPath[i++]);
            append_with_sep(_pathInfo, vreqPath, "/", i);
            break;
        }
    }
    vector<string> cgiArgs = SearchInTree(locaNode, "cgi_pass" );
    if ( cgiArgs.empty() )
        _isLocationCGI = false;
    if ( _reqExt == cgiArgs[0] )
    {
        _cgiPath = cgiArgs[1];
        _isLocationCGI = true;
    }
    _FullPath = path;
    

}

string     Path::CreatePath(AST<string> *node, string path)
{
    initData(node, path);

    int lastSize = 0;
    vector<AST<string> >& child = (*_srvNode).GetChildren();
    vector<string> vReqPath, vLocaArgPath;
    HandleRequestPath(vReqPath);
    _isCGI = IsCGI(_requestPath);
    for (int i = 0; i < (int)child.size(); i++)
    {
        if ( child[i].GetValue() == "location" )
        {
            HandleLocationArgPath(vLocaArgPath, child[i].GetArguments()[0]);
            int pos = vectorCmp(vReqPath, vLocaArgPath);
            if ( pos > lastSize )
            {
                if (  _isCGI == true )
                    HandleCGI( child[i], vReqPath );
                else
                    fillLocationInfo(child[i]);
                lastSize = pos;
            }
        }
    }
    if ( _FullPath.empty() && _isCGI == false )
        HandleSRVPath();
    else if ( _FullPath.empty() && _isCGI == true )
        HandleCGI(*_srvNode, vReqPath);
    cout << "full path befor check existance: <" << _FullPath << ">\n";
    CheckPathExist(_FullPath);
    return ( _FullPath );
}

AST<string>&   Path::getRequestNode()
{
    return ( *_requestPathNode );
}

string     Path::getLocationIndex()
{
    return ( _locaIndex );
}
string     Path::getServerIndex()
{
    return ( _srvIndex );
}

string Path::getServerPath()
{
    return ( _srvRootPath );
}
string Path::getFullPath()
{
    return ( _FullPath );
}

string Path::getPathInfo()
{
    return ( _pathInfo );
}

string Path::getExtantion()
{
    return ( _reqExt );
}
bool Path::isLocationCGi()
{
    return ( _isLocationCGI );
}

