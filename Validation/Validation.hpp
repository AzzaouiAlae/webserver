#pragma once

#include "../Headers.hpp"

class Validation {
    private:
        int     _idx;
        bool    _duplicateServer_Root;
        bool    _duplicateServer_Index;
        bool    _duplicateLocation_Root;
        bool    _duplicateLocation_Index;
        int     _level;
        std::vector<std::string> _data;
        
        typedef std::map<std::string, void (Validation::*)() >Map;
        Map _map;

        void            CreateMap();
        void            ScopValidation();
        
        void            IsValidServer();
        void            ResetServerSeting();
        
        void            IsValidLocation();
        void            ResetLocationSeting();

        void            IsValidIndex();
        void            CkeckDuplicationIndex(std::string msg);
        
        void            IsValidRoot();
        void            CkeckDuplicationRoot(std::string msg);

        void            IsValidServerName();
        bool            IsSeparator();

        void            IsValidListen();
        void            IpAndPort();
        void            PortOnly();
        void            ValidIP();
        long long       ConvertToNumber(std::string num);

    public:
    
        Validation(std::vector<std::string> inputData);
        void    CheckValidation();

};
