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

std::string     Path::LocationFullPath( AST<std::string>& currLocationNode )
{
    
    _locaRootPath = SearchInTree( currLocationNode, "root");
    if ( _locaRootPath.empty() )
        _locaFullPath = AttachPath(_srvRootPath, _locaArgPath);
    else
        _locaFullPath = AttachPath(_locaRootPath, _locaArgPath);
    return _locaFullPath;
}


void    Path::AttachIndex(AST<std::string>& currSrvNode, AST<std::string>& currLocationNode, std::string path, std::string type)
{
    _srvIndex = SearchInTree(currSrvNode, "index");
    _locaIndex = SearchInTree(currLocationNode, "index");

    if ( type == "server" )
        _targetPathWithIndex = AttachPath(path, _srvIndex);
    else if ( type == "location" )
        _targetPathWithIndex = AttachPath(path, _locaIndex);

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


/*
    prev = /a/b
    curr =  /a
    target = /a/b 


    location /a/b {
        root /var/www/html;
    }
    location /a {
        root /var/www/html;
    }
    location /a/b/c {
        root /var/www/html;
    }

*/
void        Path::fillLocationInfo(AST<std::string> & locaNode, vector<string> vLocaArgPath)
{
    _locaArgPath.clear();
    for (size_t i = 0; i < vLocaArgPath.size(); i++)
        _locaArgPath += "/" + vLocaArgPath[i];
    
    LocationFullPath( locaNode);
    _locaIndex = SearchInTree(locaNode, "index");
    _targetPathWithIndex = AttachPath(_locaFullPath, _locaIndex);
    _targetPath = _locaFullPath;
    _requestPathNode = &locaNode;
}

std::string     Path::CreatePath()
{
    int lastSize = 0;
    vector<AST<std::string> >& child = _srvNode.GetChildren();
    vector<string> vReqPath, vLocaArgPath;
    parsePath(vReqPath, _requestPath, "/");
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
            if ( pos > lastSize)
            {
                lastSize = pos;
                fillLocationInfo(child[i], vLocaArgPath);
            }
        }
    }
    if ( _targetPath.empty() )
    {
        
        _targetPathWithIndex = AttachPath(_srvRootPath, _srvIndex);
        _targetPath = _srvRootPath;
        _requestPathNode = &_srvNode;
    }
    return _targetPath;
}

AST<std::string>&   Path::getRequestNode()
{
    return *_requestPathNode;
}

std::string     Path::getFullPathWithIndex()
{
    return _targetPathWithIndex;
}
std::string     Path::getLocationIndex()
{
    return _locaIndex;
}
std::string     Path::getServerIndex()
{
    return _srvIndex;
}

std::string Path::getServerPath()
{
    return _srvRootPath;
}
std::string Path::getLocationPath()
{
    return _targetPath;
}



