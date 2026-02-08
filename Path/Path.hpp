#pragma once

#include "../Headers.hpp"

class Path {
    private:
        AST<std::string>& _srvNode;
        AST<std::string>  *_requestPathNode;
        std::string _requestPath;

        std::string     _srvRootPath;
        std::string     _locaArgPath;
        std::string     _locaRootPath;
        std::string     _locaFullPath;
        std::string     _targetPath;
        std::string     _targetPathWithIndex;   
        std::string     _srvIndex;
        std::string     _locaIndex;

        std::string     SearchInTree(AST<std::string>& node, std::string value);
        std::string     AttachPath(std::string rootPath, std::string addPath);
        std::string     LocationFullPath( AST<std::string>& currLocationNode );
        void            AttachIndex(AST<std::string>& currSrvNode, AST<std::string>& currLocationNode, std::string path, std::string type);
        void            fillLocationInfo(AST<std::string> & locaNode, vector<string> vlocaArgPath);

    public:
        Path(AST<std::string>& node, std::string path);
        std::string CreatePath();
        AST<std::string>& getRequestNode();
        std::string getFullPathWithIndex();
        std::string getLocationIndex();
        std::string getServerIndex();
        std::string getServerPath();
        std::string getLocationPath();
};
