#pragma once

#include "../Headers.hpp"

class Validation {
    private:
        int _idx;
        int _brackets;
        
        typedef std::map<std::string, void (Validation::*)() >Map;
        Map _map;

        void    CreateMap();
        void    IsValidServer();
        void    IsValidListen();
        void    IsValidIndex();
        void    IsValidRoot();
        void    IsValidServerName();
        void    IsValidLocation();
        void    ScopValidation();

    public:
    std::vector<std::string> _data;
        Validation(std::vector<std::string> inputData);
        void    CheckValidation();

};
