#pragma once

#include "../Headers.hpp"

class Path {
    private:
        AST<string> *_srvNode;
        AST<string> *_requestPathNode;
        string      _requestPath;
        string      _srvRootPath, _srvIndex;
        string      _locaRootPath, _locaArgPath, _locaIndex;
        string      _FullPath;
        string      _cgiPath, _reqExt, _pathInfo, _errorCode;
        bool        _isExtention, _isLocationCGI, _isDir, _isErrorPath;


        void            initData(AST<string> *node, string path);
        vector<string>  SearchInTree(AST<std::string>& node, std::string value );
        string          AttachPath(string rootPath, string addPath);
        string          FullPath( AST<string>& currLocationNode );
        string          AttachIndex( AST<string>& currLocationNode, string path, string type);
        string          getCodePath(AST<string> & srvNode, string srvPath, string type, string errorCode);
        void            fillLocationInfo( AST<string> & locaNode );
        void            CheckPathExist(string& path);
        void            IsRedirection(string& path);
        void            IsDirectory(struct stat info, string& path);
        void            IsFile(struct stat info, string& path);
        void            HandleSRVPath();
        void            HandleRequestPath(vector<string>& vReqPath);
        void            HandleLocationArgPath( vector<string>& vLocaArgPath, string locationArg );
        bool            IsIndexPath(string requestPath, string locArgPath);
        void            CkeckPath(vector<string>& strs);
        void            parsePath(vector<string>& strs, string& str,const string& sep);
        int             vectorCmp(vector<string>& reqPath, vector<string>&  locationPath);
        void            append_with_sep(string& result, vector<string>& vec, string sep, int pos = 0);
        bool            checkCGI(string first, string second, string& Ext);
        bool            IsExtention(string str);
        string          getExtention(string path, string p);
        string          findRootPath(AST<string>& currNode);
        void            HandleCGI( AST<string> & locaNode, vector<string>& vreqPath );
    public:
        Path();
        string          CreatePath(AST<string> *node, string path);
        AST<string>&    getRequestNode();
        string          getLocationIndex();
        string          getServerIndex();
        string          getServerPath();
        string          &getFullPath();
        string          getPathInfo();
        string          getCGiPath();
        string          getExtantion();
        bool            isLocationCGi();
		string getErrorCode();
		bool emptyRoot();
		bool IsDir();
		bool isErrorPath();
};
