#include "../Headers.hpp"


Validation::Validation(std::vector<std::string> inputData)
    :  _idx(0), _data(inputData) {}

// {"server", "{", "listen", "0.0.....0.0", ":", "80", ";", "root", "/app", ";", "index", "index.php", "index.html", ";" };

void    Validation::CreateMap()
{
    _map["server"] = &Validation::IsValidServer;
    _map["listen"] = &Validation::IsValidListen;
    _map["root"] = &Validation::IsValidRoot;
    // _map["index"] = &Validation::IsValidIndex;
    // _map["server_name"] = &Validation::IsValidServerName;
    // _map["location"] = &Validation::IsValidLocation;
    
}

void    Validation::IsValidServer()
{
    std::cout << _data[_idx] << "    " ;
    if ( _brackets != 0 || _data[++_idx] != "{" )
       Error::ErrorAndExit("Invalid Syntax");

    _brackets++;
    _idx++;
    
    ScopValidation();
}

void    Validation::IsValidListen()
{
    std::cout << _data[_idx] << "    " ;
    _idx++;
}

void    Validation::IsValidIndex()
{	
	
}

void    Validation::IsValidRoot()
{
    std::cout << _data[_idx] << "    " ;
    _idx++;
}

void    Validation::IsValidServerName()
{

}

void    Validation::IsValidLocation()
{

}

// {"server", "{", "listen", "0.0.....0.0", ":", "80", ";", "root", "/app", ";", "index", "index.php", "index.html", ";" };

// server {
//     location {

//     }
// }

void    Validation::CheckValidation()
{
    CreateMap();
    ScopValidation();
}

void    Validation::ScopValidation()
{
    while( _idx < (int)_data.size() )
    {
        std::string s = _data[_idx];
        if ( s == "}" )
        {
            if ( _brackets == 0 )
                Error::ErrorAndExit("Invalid Syntax");
            else
                return ;
        }
        Map::iterator it = _map.find(_data[_idx]);
        if ( it == _map.end() )
            Error::ErrorAndExit("Invalid Syntax");
        (this->*it->second)();
    }
    if ( _brackets != 0)
        exit(1);
    
}
