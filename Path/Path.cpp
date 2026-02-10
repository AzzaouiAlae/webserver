#include "Path.hpp"

Path::Path() : _CGI(false)
{}

void    Path::initData(AST<std::string> *node, std::string path)
{
	_srvNode = node;
	_requestPath = path;
	_srvRootPath = SearchInTree((*_srvNode), "root")[0];
    _srvIndex = SearchInTree((*_srvNode), "index")[0];
}

vector<string> Path::SearchInTree(AST<std::string>& node, std::string value )
{
	vector<AST<std::string> >& ch = node.GetChildren();
	for (int pos = 0; pos < (int)ch.size(); pos++)
	{
		if (ch[pos].GetValue() == value)
		{
			return (ch[pos].GetArguments());
		}
	}
	return vector<string>(1, "");
}

std::string     Path::AttachPath(std::string rootPath, std::string addPath)
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

bool IsIndexPath(string requestPath, string locArgPath)
{
    std::cout << "{" << requestPath << "     " << locArgPath << "}\n";
    if ( requestPath == locArgPath )
        return ( true );
    return ( false );
}

std::string     Path::FullPath( AST<std::string>& currNode )
{
    _locaRootPath = SearchInTree( currNode, "root")[0];
    string rootPath, locationPath;
    if ( _locaRootPath.empty() )
        rootPath = _srvRootPath;
    else
        rootPath = _locaRootPath;
    if ( IsIndexPath( _requestPath, _locaArgPath ) == true )
        locationPath = AttachIndex( currNode, _requestPath, "location" );
    else
        locationPath = _requestPath;
    _FullPath = AttachPath(rootPath, locationPath) ;
    std::cout << "<" << rootPath << "        " << locationPath << ">\n";
    return _FullPath;
}


std::string    Path::AttachIndex( AST<std::string>& currLocationNode, std::string path, std::string type )
{
    string pathWithIndex;
    _locaIndex = SearchInTree(currLocationNode, "index")[0];

    if ( type == "server" )
        pathWithIndex = AttachPath(path, _srvIndex);
    else if ( type == "location" )
        pathWithIndex = AttachPath(path, _locaIndex);
    return ( pathWithIndex );
}

void    CkeckPath(vector<string>& strs)
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
    }
}

void parsePath(vector<string>& strs, string& str,const string& sep)
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

int vectorCmp(vector<string>& reqPath, vector<string>&  locationPath)
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
            if (i < (int)locationPath.size())
                return 0;
            return i + 1;
        }
    }
    return i + 1;
}

void    append_with_sep(string& result, std::vector<string>& vec, string sep)
{
    if (vec.empty() )
        return ;
    vector<string>::iterator it = vec.begin() ;

    result += *it;
    ++it;
    for ( ; it != vec.end(); ++it)
        result += (sep + *it);
}

void        Path::fillLocationInfo(AST<std::string> & locaNode, vector<string> vLocaArgPath)
{
    _locaArgPath.clear();
    append_with_sep(_locaArgPath, vLocaArgPath, "/");

    FullPath( locaNode);
    _requestPathNode = &locaNode;
}

std::string Path::getErrorPagePath(AST<std::string> & srvNode, string srvPath, string errorCode)
{
    vector<AST<std::string> >& ch = srvNode.GetChildren();

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
}

void    Path::HandleSRVPath()
{
    std::cout << "'" << _requestPath << "'\n";
    if ( _requestPath.empty() )
        _FullPath = AttachPath(_srvRootPath, _srvIndex);
    _FullPath = AttachPath(_srvRootPath, _requestPath);
    _requestPathNode = &(*_srvNode);
}

bool    IsSuranded(string str, char begin, char end)
{
    if ( str[0] == begin && str[str.size() - 1] == end )
        return ( true );
    return ( false );
}

std::string     Path::CreatePath(AST<std::string> *node, std::string path)
{
    initData(node, path);

    int lastSize = 0;
    vector<AST<std::string> >& child = (*_srvNode).GetChildren();
    vector<string> vReqPath, vLocaArgPath;
    HandleRequestPath(vReqPath);
    for (int i = 0; i < (int)child.size(); i++)
    {
        if ( child[i].GetValue() == "location" )
        {
            HandleLocationArgPath(vLocaArgPath, child[i].GetArguments()[0]);
            int pos = vectorCmp(vReqPath, vLocaArgPath);
            bool isCgi = for_each(vLocaArgPath.begin(), vLocaArgPath,end(), IsSuranded);
            if ( pos > lastSize )
            {
                lastSize = pos;
                fillLocationInfo(child[i], vLocaArgPath);
            }
        }
    }
    if ( _FullPath.empty() )
        HandleSRVPath();
    cout << "full path befor check existance: <" << _FullPath << ">\n";
    CheckPathExist(_FullPath);
    return ( _FullPath );
}

AST<std::string>&   Path::getRequestNode()
{
    return ( *_requestPathNode );
}

std::string     Path::getLocationIndex()
{
    return ( _locaIndex );
}
std::string     Path::getServerIndex()
{
    return ( _srvIndex );
}

std::string Path::getServerPath()
{
    return ( _srvRootPath );
}
std::string Path::getFullPath()
{
    return ( _FullPath );
}



