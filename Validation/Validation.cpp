#include "Validation.hpp"

// {"server", "{", "listen", "0.0.....0.0", ":", "80", ";", "root", "/app", ";", "index", "index.php", "index.html", ";" };

Validation::Validation(std::vector<std::string> inputData)
    :  _idx(0), _data(inputData) {}


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
    this->_brackets++;
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
        Map::iterator it = this->_map.find(_data[_idx]);
        if ( it == _map.end() || !(this->*it->second)() )
            return (false);
    }
    if ( this->_brackets != 0)
        return (false);
    return true;
}
