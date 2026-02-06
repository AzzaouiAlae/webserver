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
    _locaArgPath = currLocationNode.GetArguments()[0];
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

std::string     Path::CreatePath()
{
    std::string prevlocaPath;
    vector<AST<std::string> >& child = _srvNode.GetChildren();

    for (int i = 0; i < (int)child.size(); i++)
    {
        if ( child[i].GetValue() == "location" )
        {
            LocationFullPath( child[i]);
            int posFoundInrequest = _requestPath.find(_locaFullPath);
            int posFoundInlocaPath = _locaFullPath.find(_requestPath);
            if ( (posFoundInrequest == 0 ||  posFoundInlocaPath == 0) && _locaFullPath.size() > prevlocaPath.size() )
            {
                _locaIndex = SearchInTree(child[i], "index");
                _targetPathWithIndex = AttachPath(_locaFullPath, _locaIndex);
                _targetPath = _locaFullPath;
                _requestPathNode = &child[i];
                prevlocaPath = _locaFullPath;
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
