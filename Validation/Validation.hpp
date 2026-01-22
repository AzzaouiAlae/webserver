#pragma once

#include "../Headers.hpp"

class Validation {
    private:
        int _idx;
        int _brackets;
        
        typedef std::map<std::string, void (Validation::*)() >Map;
        Map _map;

        void            CreateMap();
        void            IsValidServer();
        void            IsValidIndex();
        void            IsValidRoot();
        void            IsValidLocation();
        void            ScopValidation();
        
        void            IsValidServerName();
        bool            IsSeparator();

        void            IsValidListen();
        void            IpAndPort();
        void            PortOnly();
        void            ValidIP();
        long long       ConvertToNumber(std::string num);

    public:
    std::vector<std::string> _data;
        Validation(std::vector<std::string> inputData);
        void    CheckValidation();

};
