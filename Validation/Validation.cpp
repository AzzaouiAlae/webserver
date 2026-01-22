#include "../Headers.hpp"

Validation::Validation(std::vector<std::string> inputData)
    :  _idx(0), _brackets(0), _data(inputData) {}

void    Validation::CreateMap()
{
    _map["server"] = &Validation::IsValidServer;
    _map["listen"] = &Validation::IsValidListen;
    _map["server_name"] = &Validation::IsValidServerName;
    // _map["root"] = &Validation::IsValidRoot;
    // _map["index"] = &Validation::IsValidIndex;
    // _map["location"] = &Validation::IsValidLocation;
    
}

void    Validation::IsValidServer()
{
    std::cout << _data[_idx] << "    \n" ;
    _idx++;
    if ( _brackets != 0 || _data[_idx] != "{" )
       Error::ThrowError("Invalid Syntax");
    else
        _brackets++;
    _idx++;
    
    ScopValidation();
}

long long   Validation::ConvertToNumber(std::string num)
{
    char* endptr;
    errno = 0;

    long long port = std::strtoll(num.c_str(), &endptr, 10);

    if ( errno != 0 || *endptr != '\0' || num[0] == '+' )
        Error::ThrowError("Invalid Syntax ( Number Not Valid )");
    return ( port );
}

void Validation::PortOnly()
{
    long long port = ConvertToNumber(_data[_idx]);
    std::cout << _data[_idx] << "    \nport =="  << port;
    
    if ( port < 0 || port > 65535 )
        Error::ThrowError("Invalid Syntax ( Port Number Out Of Range )");

    _idx++;
}

void    Validation::ValidIP()
{
    long long Ip = 0;
    size_t countPoint = 0;
    size_t start = 0;
    size_t pos  = 0;
    while ( ( pos = _data[_idx].find('.', start) ) != std::string::npos )
    {
        if ( countPoint >= 3)
            Error::ThrowError("Invalid Syntax ( IP Address Not Valid )");
        countPoint++;
        Ip = ConvertToNumber( _data[_idx].substr(start, pos - start));

        if ( Ip < 0 || Ip > 255)
            Error::ThrowError("Invalid Syntax ( IP Adress: Number Out Of Range )");
        start = pos + 1;
    }
    pos = *(_data[_idx].end() - 1);
    Ip = ConvertToNumber( _data[_idx].substr(start, pos - start));
        if ( Ip < 0 || Ip > 255)
            Error::ThrowError("Invalid Syntax ( IP Adress: Number Out Of Range )");
    if ( countPoint != 3 )
        Error::ThrowError("Invalid Syntax ( IP Address Not Valid )");
    _idx++;
}

void Validation::IpAndPort()
{
    ValidIP();
    std::cout << _data[_idx] << "    \n" ;
    _idx++;
    PortOnly();
}

void    Validation::IsValidListen()
{
    std::cout << _data[_idx] << "    \n" ;
    _idx++;

    if (  _idx + 1 < (int)_data.size() )
    {
        if (_data[_idx + 1] == ":" )
            IpAndPort();
        else if ( _data[_idx + 1] == ";"  )
            PortOnly();
        else
            Error::ThrowError("Invalid Syntax");
        _idx++;
    }
    else
        Error::ThrowError("Invalid Syntax");

}

bool Validation::IsSeparator()
{
    if ( _data[_idx] == ";" ||  _data[_idx] == "}" || _data[_idx] == "{")
        return ( true );
    return ( false );
}

void    Validation::IsValidServerName()
{
    std::cout << _data[_idx] << "    \n" ;
    _idx++;

    if ( _data[_idx] == ";" )
        Error::ThrowError("Invalid Syntax : (server_name Must Have a Name)");
    while ( _idx < (int)_data.size() && !IsSeparator() )
        _idx++;
    if (  _data[_idx] == ";" )
        _idx++;
}

void    Validation::IsValidIndex()
{	
	
}

void    Validation::IsValidRoot()
{
    std::cout << _data[_idx] << "    \n" ;
    _idx++;
}

void    Validation::IsValidLocation()
{

}



void    Validation::CheckValidation()
{
    CreateMap();
    ScopValidation();
}

void    Validation::ScopValidation()
{
    while( _idx < (int)_data.size() )
    {
        if (  _data[this->_idx] == "}" )
        {
            if ( _brackets == 0 )
                Error::ThrowError("Invalid Syntax");
            else
            {
                _brackets--;
                _idx++;
                return ;
            }
        }
        Map::iterator it = _map.find(_data[_idx]);
        if ( it == _map.end() )
            Error::ThrowError("Invalid Syntax");
        (this->*it->second)();
    }
    if ( _brackets != 0)
        Error::ThrowError("Invalid Syntax");

}
