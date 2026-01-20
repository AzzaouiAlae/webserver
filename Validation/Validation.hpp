#ifndef VALIDATION_HPP
#define VALIDATION_HPP

#include "../AbstractSyntaxTree/AST.hpp"
#include "map"

class Validation {
    private:
        int _idx;
        int _brackets;
        std::vector<std::string> _data;
        std::map<std::string, bool (Validation::*)() >_map;

        void    CreateMap();
        bool    IsValidServer();
        bool    IsValidListen();
        bool    IsValidIndex();
        bool    IsValidRoot();
        bool    IsValidServerName();
        bool    IsValidLocation();

    public:
        Validation(std::vector<std::string> inputData);
        bool    CheckValidation();

};

#endif