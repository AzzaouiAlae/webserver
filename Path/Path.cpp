#include "Path.hpp"

Path::Path() : _isExtention(false), _isLocationCGI(false)
{
	_isDir = false;
	_isErrorPath = false;
}

void Path::initData(AST<string> *node, string path)
{
	_srvNode = node;
	_requestPath = path;
	_srvRootPath = SearchInTree((*_srvNode), "root")[0];
	_srvIndex = SearchInTree((*_srvNode), "index")[0];
}

vector<string> Path::SearchInTree(AST<string> &node, string value)
{
	vector<AST<string> > &ch = node.GetChildren();
	for (int pos = 0; pos < (int)ch.size(); pos++)
	{
		if (ch[pos].GetValue() == value)
		{
			return (ch[pos].GetArguments());
		}
	}
	return vector<string>(1, "");
}

string Path::AttachPath(string rootPath, string addPath)
{
	if (rootPath.empty() && addPath.empty())
	{
		return "";
	}
	if (rootPath == "")
	{
		rootPath = "/";
	}
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
	if (requestPath == locArgPath)
		return (true);
	return (false);
}

string Path::findRootPath(AST<string> &currNode)
{
	_locaRootPath = SearchInTree(currNode, "root")[0];

	if (_locaRootPath.empty())
		return (_srvRootPath);

	return (_locaRootPath);
}

string Path::FullPath(AST<string> &currNode)
{
	string rootPath, locationPath;
	rootPath = findRootPath(currNode);
	if (IsIndexPath(_requestPath, _locaArgPath) == true)
		locationPath = AttachIndex(currNode, _requestPath, "location");
	else
	{
		locationPath = _requestPath;
	}
	if (locationPath != "")
		_FullPath = AttachPath(rootPath, locationPath);
	return _FullPath;
}

string Path::AttachIndex(AST<string> &currLocationNode, string path, string type)
{
	string pathWithIndex;
	_locaIndex = SearchInTree(currLocationNode, "index")[0];

	if (type == "server")
		pathWithIndex = AttachPath(path, _srvIndex);
	else if (type == "location")
		pathWithIndex = AttachPath(path, _locaIndex);
	return (pathWithIndex);
}

void DecodeString(string &str)
{
	Logging::Debug() << "the encoded string befor decode: " << str;

	string result;
	string::size_type pos = 0;
	string::size_type start = 0;

	while ((pos = str.find('%', start)) != string::npos)
	{
		result += str.substr(start, pos - start);

		if (pos + 2 < str.size() &&
			Utility::isHexa(str.substr(pos + 1, 2)))
		{
			string hex = str.substr(pos + 1, 2);
			result += Utility::HexaToChar(hex);
			start = pos + 3;
		}
		else
		{
			result += '%';
			start = pos + 1;
		}
	}

	if (start < str.size())
		result += str.substr(start);

	if (!result.empty())
		str = result;
	Logging::Debug() << "the decoded string after: " << str;
}

void Path::CkeckPath(vector<string> &strs)
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
		else if (strs[i] == ".")
			strs.erase(strs.begin() + i);
		else if (strs[i].find('%') != string::npos)
			DecodeString(strs[i]);
	}
}

void Path::parsePath(vector<string> &strs, string &str, const string &sep)
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

int Path::vectorCmp(vector<string> &reqPath, vector<string> &locationPath)
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
			if (i < (int)locationPath.size() && _isExtention == true && locationPath[i] != _reqExt)
				return 0;
			return i + 1;
		}
	}
	return i + 1;
}

void Path::append_with_sep(string &result, vector<string> &vec, string sep, int pos)
{
	if (vec.empty())
		return;

	if (pos > (int)vec.size())
		return;
	result += vec[pos];
	pos++;
	for (; pos < (int)vec.size(); pos++)
		result += (sep + vec[pos]);
}

void Path::fillLocationInfo(AST<string> &locaNode)
{
	Logging::Debug() << "Fill location info : " << locaNode.GetValue();
	FullPath(locaNode);
	_requestPathNode = &locaNode;
}

string Path::getCodePath(AST<string> &srvNode, string srvPath, string type, string errorCode)
{
	vector<AST<string> > &ch = srvNode.GetChildren();

	for (int pos = 0; pos < (int)ch.size(); pos++)
	{
		
		if (ch[pos].GetValue() == type)
		{
			vector<string> errorpages = ch[pos].GetArguments();
			if (!errorpages.empty() && find(errorpages.begin(), errorpages.end(), errorCode) != errorpages.end()) {
				_isErrorPath = true;
				return (AttachPath(srvPath, errorpages[errorpages.size() - 1]));
			}
		}
	}
	_isErrorPath = false;
	return (errorCode);
}

bool Path::isErrorPath()
{
	return _isErrorPath;
}

bool Path::IsDir()
{
	return _isDir;
}

void Path::IsDirectory(struct stat info, string &path)
{
	if (S_ISDIR(info.st_mode))
	{
		if (access(path.c_str(), R_OK))
		{
			path = getCodePath((*_srvNode), _srvRootPath, "error_page", "403");
			_errorCode = "403";
			Error::ThrowError(path);
		}
		vector<string> autoindex = SearchInTree(*_srvNode, "autoindex");
		if (autoindex.size() == 0 || autoindex[0] == "" || autoindex[0] == "off")
		{
			autoindex = SearchInTree(*_requestPathNode, "autoindex");
		}
		if (autoindex.size() == 0 || autoindex[0] == "" || autoindex[0] == "off")
		{
			path = getCodePath((*_srvNode), _srvRootPath, "error_page", "403");
			_errorCode = "403";
			Error::ThrowError(path);
		}
		_isDir = true;
	}
}

void Path::IsFile(struct stat info, string &path)
{
	if (S_ISREG(info.st_mode))
	{
		int fd = open(path.c_str(), O_RDONLY);
		if (fd < 0)
		{
			path = getCodePath((*_srvNode), _srvRootPath, "error_page", "403");
			_errorCode = "403";
			Error::ThrowError(path);
		}
		close(fd);
	}
}

void Path::IsRedirection(string &path)
{
	(void)path;
	// for (int i = 0; i < 3 ; i++)
	// {

	// }

	// getCodePath(*_requestPathNode, _locaRootPath, "return", to_string(i + 300));
}

void Path::CheckPathExist(string &path)
{
	struct stat info;
	if (stat(path.c_str(), &info) != 0)
	{
		path = getCodePath((*_srvNode), _srvRootPath, "error_page", "404");
		_errorCode = "404";
		Error::ThrowError(path);
	}
	IsDirectory(info, path);
	IsFile(info, path);
	// IsRedirection(path);
}

void Path::HandleRequestPath(vector<string> &vecReqPath)
{
	parsePath(vecReqPath, _requestPath, "/");
	_requestPath.clear();
	append_with_sep(_requestPath, vecReqPath, "/");
}

void Path::HandleLocationArgPath(vector<string> &vLocaArgPath, string locationArg)
{
	vLocaArgPath.clear();
	_locaArgPath = locationArg;
	if (_locaArgPath[_locaArgPath.size() - 1] != '/')
		_locaArgPath += "/";
	parsePath(vLocaArgPath, _locaArgPath, "/");
	_locaArgPath.clear();
	append_with_sep(_locaArgPath, vLocaArgPath, "/");
}

void Path::HandleSRVPath()
{
	if (_requestPath.empty())
		_FullPath = AttachPath(_srvRootPath, _srvIndex);
	else
		_FullPath = AttachPath(_srvRootPath, _requestPath);
	_requestPathNode = &(*_srvNode);
	Logging::Debug() << "Path Handle SRV Path: srvIndex:" << _srvIndex;
}

string Path::getExtention(string path, string p)
{
	string::size_type point = path.find_last_of(p);
	if (point == string::npos || (point + 1) >= path.size())
		return ("");
	if (path[point + 1] == '/')
		return "";
	string::size_type slash = path.find('/', point);
	if (slash == string::npos)
		slash = path.size();

	return (path.substr(point, slash - point));
}

bool Path::checkCGI(string first, string second, string &Extention)
{
	string reqExt = getExtention(first, ".");
	string locaExt = getExtention(second, ".");
	if (reqExt.empty() || locaExt.empty())
		return (false);
	if (reqExt != locaExt)
		return (false);
	Extention = reqExt;
	return (true);
}

bool Path::IsExtention(string str)
{
	string reqExt = getExtention(str, ".");
	if (reqExt.empty())
		return (false);
	_reqExt = reqExt;
	return (true);
}

bool Path::emptyRoot()
{
	return _srvRootPath == "" && _locaRootPath == "";
}

void Path::HandleCGI(AST<string> &locaNode, vector<string> &vreqPath)
{
	string extention, path;
	_requestPathNode = &locaNode;
	path = findRootPath(locaNode);
	Logging::Debug() << "Try to HandleCGI Path";
	for (size_t i = 0; i < vreqPath.size(); i++)
	{
		extention = getExtention(vreqPath[i], ".");
		Logging::Debug() << "get Request Extention: " << extention;
		if (extention.empty())
			path = AttachPath(path, vreqPath[i]);
		else
		{
			path = AttachPath(path, vreqPath[i++]);
			if (i < vreqPath.size())
				append_with_sep(_pathInfo, vreqPath, "/", i);
			break;
		}
	}
	Logging::Debug() << "Found CGI path: " << (_requestPathNode->GetArguments())[0];

	vector<string> cgiArgs = SearchInTree(locaNode, "cgi_pass");
	if (cgiArgs.empty())
		_isLocationCGI = false;
	if (_reqExt == cgiArgs[0])
	{
		_cgiPath = cgiArgs[1];
		_isLocationCGI = true;
	}
	_FullPath = path;
}

string Path::CreatePath(AST<string> *node, string path)
{
	// path = "/app/hex%F0%9F%98%83in%20the%20midle%20%D9%85%D8%B1%D8%AD%D8%A7%D8%A7%D8%A7%D8%A7%D8%A7%D8%A7%D8%A7%D8%A7/";
	Logging::Debug() << "request path in create path is: " << path;
	initData(node, path);

	int lastSize = 0;
	vector<AST<string> > &child = (*_srvNode).GetChildren();
	vector<string> vReqPath, vLocaArgPath;
	HandleRequestPath(vReqPath);
	_isExtention = IsExtention(_requestPath);
	Logging::Debug() << "Path create reqPath list with size: " << vReqPath.size()
					 << " Path isCGI: " << _isExtention;
	for (int i = 0; i < (int)child.size(); i++)
	{

		if (child[i].GetValue() == "location")
		{
			HandleLocationArgPath(vLocaArgPath, child[i].GetArguments()[0]);
			int pos = vectorCmp(vReqPath, vLocaArgPath);
			if (pos > lastSize)
			{
				Logging::Debug() << "current location path: " << _locaArgPath;
				if (_isExtention == true)
					HandleCGI(child[i], vReqPath);
				else
					fillLocationInfo(child[i]);
				lastSize = pos;
			}
		}
	}
	if (_FullPath.empty() && _isExtention == false)
		HandleSRVPath();
	else if (_FullPath.empty() && _isExtention == true)
		HandleCGI(*_srvNode, vReqPath);

	Logging::Debug() << "full path befor check existance: <" << _FullPath;
	CheckPathExist(_FullPath);
	Logging::Debug() << "path created in create path " << _FullPath << "        and path info: " << _pathInfo;
	return (_FullPath);
}

AST<string> &Path::getRequestNode()
{
	return (*_requestPathNode);
}

string Path::getLocationIndex()
{
	return (_locaIndex);
}

string Path::getServerIndex()
{
	return (_srvIndex);
}

string Path::getServerPath()
{
	return (_srvRootPath);
}

string &Path::getFullPath()
{
	return (_FullPath);
}

string Path::getPathInfo()
{
	return (_pathInfo);
}

string Path::getExtantion()
{
	return (_reqExt);
}

bool Path::isLocationCGi()
{
	return (_isLocationCGI);
}

string Path::getErrorCode()
{
	return _errorCode;
}
