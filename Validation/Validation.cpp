#include "Validation.hpp"

// {"server", "{", "listen", "0.0.....0.0", ":", "80", ";", "root", "/app", ";", "index", "index.php", "index.html", ";" };

Validation::Validation(std::vector<std::string> inputData)
    :  _idx(0), _data(inputData) {}


void    Validation::CreateMap()
{
    _map["server"] = &Validation::IsValidServer;
    _map["listen"] = &Validation::IsValidListen;
    _map["root"] = &Validation::IsValidRoot;
    _map["index"] = &Validation::IsValidIndex;
    _map["server_name"] = &Validation::IsValidServerName;
    _map["location"] = &Validation::IsValidLocation;
    
}

bool    Validation::IsValidServer()
{
    this->_brackets++;
	return false;
}

bool    Validation::IsValidListen()
{
	return false;

}

bool    Validation::IsValidIndex()
{	
	return false;

}

bool    Validation::IsValidRoot()
{
	return false;

}

bool    Validation::IsValidServerName()
{
	return false;

}

bool    Validation::IsValidLocation()
{
	return false;

}

bool    Validation::CheckValidation()
{
    CreateMap();

    while( _idx++ < (int)_data.size())
    {
        Map::iterator it = _map.find(_data[_idx]);
        if ( it == _map.end() || !(this->*it->second)() )
            return (false);
    }
    if ( _brackets != 0)
        return (false);
    return true;
}
