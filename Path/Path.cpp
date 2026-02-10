#include "Path.hpp"

Path::Path(AST<string> *node, string &path) : _srvNode(node), _requestPath(path)
{
	_srvRootPath = SearchInTree(*_srvNode, "root");
	_srvIndex = SearchInTree(*_srvNode, "index");
}

Path::Path()
{}

void Path::Find(AST<string> *node, string &path)
{
	_requestPath = path;
	_srvNode = node;
	_srvRootPath = SearchInTree(*_srvNode, "root");
	_srvIndex = SearchInTree(*_srvNode, "index");
}

string Path::SearchInTree(AST<string> &node, string value)
{
	vector<AST<string> > &ch = node.GetChildren();
	for (int i = 0; i < (int)ch.size(); i++)
	{
		if (ch[i].GetValue() == value)
		{
			return (ch[i].GetArguments())[0];
		}
	}
	return "";
}

string Path::AttachPath(string rootPath, string addPath)
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

string Path::LocationFullPath(AST<string> &currLocationNode)
{

	_locaRootPath = SearchInTree(currLocationNode, "root");
	if (_locaRootPath.empty())
		_locaFullPath = AttachPath(_srvRootPath, _locaArgPath);
	else
		_locaFullPath = AttachPath(_locaRootPath, _locaArgPath);
	return _locaFullPath;
}

void Path::AttachIndex(AST<string> &currSrvNode, AST<string> &currLocationNode, string path, string type)
{
	_srvIndex = SearchInTree(currSrvNode, "index");
	_locaIndex = SearchInTree(currLocationNode, "index");

	if (type == "server")
		_targetPathWithIndex = AttachPath(path, _srvIndex);
	else if (type == "location")
		_targetPathWithIndex = AttachPath(path, _locaIndex);
}

void CkeckPath(vector<string> &strs)
{
	if (strs.empty() || strs.size() == 1)
		return;

	for (int i = 0; i < (int)strs.size(); i++)
	{
		if (strs[i] == "..")
		{
			strs.erase(strs.begin() + i);
			if (i > 0)
				strs.erase(strs.begin() + (i - 1));
			i = -1;
		}
	}
}

void parsePath(vector<string> &strs, string &str, const string &sep)
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

int vectorCmp(vector<string> &reqPath, vector<string> &locationPath)
{
	if (locationPath.size() > reqPath.size())
		return 0;
	if (locationPath.size() == 0)
		return 1;
	int i;
	for (i = 0; i < (int)reqPath.size(); i++)
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

void Path::fillLocationInfo(AST<string> &locaNode, vector<string> vLocaArgPath)
{
	_locaArgPath.clear();
	for (size_t i = 0; i < vLocaArgPath.size(); i++)
		_locaArgPath += "/" + vLocaArgPath[i];

	LocationFullPath(locaNode);
	_locaIndex = SearchInTree(locaNode, "index");
	_targetPathWithIndex = AttachPath(_locaFullPath, _locaIndex);
	_targetPath = _locaFullPath;
	_requestPathNode = &locaNode;
}

string Path::CreatePath()
{
	int lastSize = 0;
	vector<AST<string> > &child = _srvNode->GetChildren();
	vector<string> vReqPath, vLocaArgPath;
	parsePath(vReqPath, _requestPath, "/");
	for (int i = 0; i < (int)child.size(); i++)
	{
		if (child[i].GetValue() == "location")
		{
			vLocaArgPath.clear();
			_locaArgPath = child[i].GetArguments()[0];
			if (_locaArgPath[_locaArgPath.size() - 1] != '/')
				_locaArgPath += "/";
			parsePath(vLocaArgPath, _locaArgPath, "/");
			int pos = vectorCmp(vReqPath, vLocaArgPath);
			if (pos > lastSize)
			{
				lastSize = pos;
				fillLocationInfo(child[i], vLocaArgPath);
			}
		}
	}
	if (_targetPath.empty())
	{
		_targetPathWithIndex = AttachPath(_srvRootPath, _srvIndex);
		_targetPath = _srvRootPath;
		_requestPathNode = _srvNode;
	}
	return _targetPath;
}

AST<string> &Path::getRequestNode()
{
	return *_requestPathNode;
}

string Path::getFullPathWithIndex()
{
	return _targetPathWithIndex;
}
string Path::getLocationIndex()
{
	return _locaIndex;
}
string Path::getServerIndex()
{
	return _srvIndex;
}

string Path::getServerPath()
{
	return _srvRootPath;
}
string Path::getLocationPath()
{
	return _targetPath;
}
