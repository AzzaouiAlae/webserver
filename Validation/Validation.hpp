#pragma once

#include "../Headers.hpp"

class Validation {
    private:
        int     _idx;
        int     _level;
        std::map<std::string, bool> _useServer;
        std::map<std::string, bool> _useLocation;
        std::vector<std::string> _data;
        
        static std::vector<std::string> _skiped;
        
        typedef std::map<std::string, void (Validation::*)() >Map;
        Map _map;
        
        static void     CheckExistance(std::pair<std::string, bool>);
        static bool     SkipedOptions(std::string option);
        void            AddDirective( std::string value );

        void            CreateMap();
        bool            IsByteSizeUnit( std::string& data );
        void            CreateServerdMap();
        void            CreateLocationMap();
        void            CreateSkipedData();
        
        void            IsValidServer();
        void            ResetServerSeting();

        void            IsValidLocation();
        void            ResetLocationSeting();

        void            IsClientMaxBodySize();

        void            IsValidIndex();
        void            IsValidRoot();
        void            IsValidAutoindex();
        void            CkeckDuplication(bool& first, bool& second, std::string msg);
        void            CheckLevelAndDuplication(bool& first, bool& second, std::string msg);
        
        void            IsValidReturn();
        void            IsValidServerName();
        bool            IsSeparator();

        void            IsErrorPage();
        void            IsValidCGIPass();

        void            IsValidAllowMethods();
        bool            IsAllowedMethods(std::string& method);

        void            IsValidListen();
        void            IpAndPort();
        void            PortOnly();
        void            ValidIP();
        long            ConvertToNumber(std::string num);

    public:
    
        Validation(std::vector<std::string> inputData);
        void    CheckValidation();

};
