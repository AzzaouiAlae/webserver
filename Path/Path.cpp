#include "Path.hpp"

Path::Path(AST<std::string>& node, std::string path) : _srvNode(node), _requestPath(path)
{
        _srvRootPath = SearchInTree(_srvNode, "root");
        _srvIndex = SearchInTree(_srvNode, "index");
}

std::string Path::SearchInTree(AST<std::string>& node, std::string value)
{
	vector<AST<std::string> >& ch = node.GetChildren();
	for (int i = 0; i < (int)ch.size(); i++)
	{
		if (ch[i].GetValue() == value)
		{
			return (ch[i].GetArguments())[0];
		}
	}
	return "";
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
    _locaRootPath = SearchInTree( currNode, "root");
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
    _locaIndex = SearchInTree(currLocationNode, "index");

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

std::string     Path::CreatePath()
{
    int lastSize = 0;
    vector<AST<std::string> >& child = _srvNode.GetChildren();
    vector<string> vReqPath, vLocaArgPath;
    parsePath(vReqPath, _requestPath, "/");
    _requestPath.clear();
    append_with_sep(_requestPath, vReqPath, "/");
    for (int i = 0; i < (int)child.size(); i++)
    {
        if ( child[i].GetValue() == "location" )
        {
            vLocaArgPath.clear();
            _locaArgPath = child[i].GetArguments()[0];
            if ( _locaArgPath[_locaArgPath.size() - 1] != '/' )
                _locaArgPath += "/";
            parsePath(vLocaArgPath, _locaArgPath, "/");
            int pos = vectorCmp(vReqPath, vLocaArgPath);
            if ( pos > lastSize )
            {
                lastSize = pos;
                fillLocationInfo(child[i], vLocaArgPath);
            }
        }
    }
    if ( _FullPath.empty() )
    {
        std::cout << "'" << _requestPath << "'\n";
        if ( _requestPath.empty() )
            _FullPath = AttachPath(_srvRootPath, _srvIndex);
        _FullPath = AttachPath(_srvRootPath, _requestPath);
        _requestPathNode = &_srvNode;
    }
    Error::errorType = NotFound;
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



