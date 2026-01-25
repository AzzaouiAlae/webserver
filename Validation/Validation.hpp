#pragma once

#include "../Headers.hpp"

class Validation {
    private:
        int     _idx;
        int     _level;
        std::map<std::string, bool> _useServer;
        std::map<std::string, bool> _useLocation;
        std::vector<std::string> _data;
        
        typedef std::map<std::string, void (Validation::*)() >Map;
        Map _map;
        
        static void     CheckExistance(std::pair<std::string, bool>);
        void            CreateMap();
        void            createServerdMap();
        void            createLocationMap();
        
        void            IsValidServer();
        void            ResetServerSeting();
        
        void            IsValidLocation();
        void            ResetLocationSeting();
        
        void            IsValidReturn();
        void            CheckURL();
        
        void            IsValidIndex();
        void            IsValidRoot();
        void            IsValidAutoindex();
        void            CkeckDuplication(bool& first, bool& second, std::string msg);
        void            CheckLevelAndDuplication(bool& first, bool& second, std::string msg);

        void            IsValidServerName();
        bool            IsSeparator();


        void            IsValidListen();
        void            IpAndPort();
        void            PortOnly();
        void            ValidIP();
        long            ConvertToNumber(std::string num);

    public:
    
        Validation(std::vector<std::string> inputData);
        void    CheckValidation();

};
