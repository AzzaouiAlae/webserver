#pragma once

#include "../Headers.hpp"

class Path
{
private:
	AST<string> *_srvNode;
	AST<string> *_requestPathNode;
	string _requestPath;

	string _srvRootPath;
	string _locaArgPath;
	string _locaRootPath;
	string _locaFullPath;
	string _targetPath;
	string _targetPathWithIndex;
	string _srvIndex;
	string _locaIndex;

	string SearchInTree(AST<string> &node, string value);
	string AttachPath(string rootPath, string addPath);
	string LocationFullPath(AST<string> &currLocationNode);
	void AttachIndex(AST<string> &currSrvNode, AST<string> &currLocationNode, string path, string type);
	void fillLocationInfo(AST<string> &locaNode, vector<string> vlocaArgPath);

public:
	Path(AST<string> *node, string &path);
	Path();
	void Find(AST<string> *node, string &path);
	string CreatePath();
	AST<string> &getRequestNode();
	string getFullPathWithIndex();
	string getLocationIndex();
	string getServerIndex();
	string getServerPath();
	string getLocationPath();
};
