#include "Validation.hpp"

// {"server", "{", "listen", "0.0.....0.0", ":", "80", ";", "root", "/app", ";", "index", "index.php", "index.html", ";" };

Validation::Validation(std::vector<std::string> inputData)
    : _data(inputData), _idx(0) {}


void    Validation::CreateMap()
{
    _map["server"] = IsValidServer;
    _map["listen"] = IsValidListen;
    _map["root"] = IsValidRoot;
    _map["index"] = IsValidIndex;
    _map["server_name"] = IsValidServerName;
    _map["location"] = IsValidLocation;
    
}

bool    Validation::IsValidServer()
{

}

bool    Validation::IsValidListen()
{

}

bool    Validation::IsValidIndex()
{

}

bool    Validation::IsValidRoot()
{

}

bool    Validation::IsValidServerName()
{

}

bool    Validation::IsValidLocation()
{

}

bool    Validation::CheckValidation()
{
    CreateMap();

    while( _idx++ < this->_data.size())
    {
        this->_map[_data[_idx]];
    }

    return true;
}
