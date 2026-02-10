#pragma once

#include "../Headers.hpp"

enum PathType {
    Error,
    Dir,
    File
};

class Path {
    private:
        AST<string> *_srvNode;
        AST<string> *_requestPathNode;
        string      _requestPath;
        string      _srvRootPath, _srvIndex;
        string      _locaRootPath, _locaArgPath, _locaIndex;
        string      _FullPath;
        PathType    _pathType;
        bool        _CGI;

        void            initData(AST<string> *node, string path);
        vector<string>  SearchInTree(AST<std::string>& node, std::string value );
        string          AttachPath(string rootPath, string addPath);
        string          FullPath( AST<string>& currLocationNode );
        string          AttachIndex( AST<string>& currLocationNode, string path, string type);
        string          getErrorPagePath(AST<std::string> & srvNode, string srvPath, string errorCode);
        void            fillLocationInfo(AST<string> & locaNode, vector<string> vlocaArgPath);
        void            CheckPathExist(string& path);
        void            IsDirectory(struct stat info, string& path);
        void            IsFile(struct stat info, string& path);
        void            HandleSRVPath();
        void            HandleRequestPath(vector<string>& vReqPath);
        void            HandleLocationArgPath( vector<string>& vLocaArgPath, string locationArg );
    public:
        Path();
        string          CreatePath(AST<string> *node, string path);
        AST<string>&    getRequestNode();
        string          getLocationIndex();
        string          getServerIndex();
        string          getServerPath();
        string          getFullPath();
};
