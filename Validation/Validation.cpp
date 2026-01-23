#include "../Headers.hpp"

Validation::Validation(std::vector<std::string> inputData)
    :  _idx(0), _level(0), _data(inputData) 
{
    createUsedMap();
}

void    Validation::createUsedMap()
{
    _used["Server"] = false;
    _used["Listen"] = false;
    _used["Server_name"] = false;
    _used["RootServer"] = false;
    _used["IndexServer"] = false;
    _used["Location"] = false;
    _used["RootLocation"] = false;
    _used["IndexLocation"] = false;
}

void    Validation::CreateMap()
{
    _map["server"] = &Validation::IsValidServer;
    _map["listen"] = &Validation::IsValidListen;
    _map["server_name"] = &Validation::IsValidServerName;
    _map["root"] = &Validation::IsValidRoot;
    _map["index"] = &Validation::IsValidIndex;
    _map["location"] = &Validation::IsValidLocation;
    
}

void Validation::CheckExistance(std::pair<std::string, bool> used)
{
    if (used.first !=  "IndexLocation" && used.first !=  "RootLocation" && used.second == false)
       Error::ThrowError("Invalid Syntax : (Missing Element in Server)");
}

void    Validation::ResetServerSeting()
{
    std::for_each(_used.begin(), _used.end(), CheckExistance);

    _used["RootServer"] = false;
    _used["IndexServer"] = false;
}
void    Validation::ResetLocationSeting()
{
    _used["RootLocation"] = false;
    _used["IndexLocation"] = false;
}

void    Validation::IsValidServer()
{
    _used["Server"] = true;
    _idx++;
    if ( _level != 0 || _data[_idx] != "{" )
       Error::ThrowError("Invalid Syntax");
    else
        _level++;
    _idx++;
    
    ScopValidation();
    ResetServerSeting();

}

void    Validation::IsValidLocation()
{
    _used["Location"] = true;
    _idx++;
    if ( _level != 1 || IsSeparator() )
       Error::ThrowError("Invalid Syntax");
    _idx++;
    if ( _data[_idx] != "{" )
       Error::ThrowError("Invalid Syntax");
    else
        _level++;
    _idx++;
    
    ScopValidation();
    ResetLocationSeting();
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
    _idx++;
    PortOnly();
}

void    Validation::IsValidListen()
{
    _used["Listen"] = true;
    if ( _level != 1)
        Error::ThrowError("Invalid Syntax : (Invalid Scoop)");
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
    _used["Server_name"] = true;
    if ( _level != 1)
        Error::ThrowError("Invalid Syntax : (Invalid Scoop)");
    _idx++;

    if ( _data[_idx] == ";" )
        Error::ThrowError("Invalid Syntax : (server_name Must Have a Name)");
    while ( _idx < (int)_data.size() && !IsSeparator() )
        _idx++;
    if (  _data[_idx] == ";" )
        _idx++;
}

void    Validation::CkeckDuplication(bool& first, bool& second, std::string msg)
{
    if ( _level == 1 )
    {
        if (first  == true )
            Error::ThrowError(msg);
        else
            first = true;
        return ;
    }
    else if (_level == 2)
    {
        if (second  == true )
            Error::ThrowError(msg);
        else
            second = true;
        return ;
    }
    else
        Error::ThrowError("Invalid Syntax : (Invalid Scoop)");
}

void    Validation::IsValidRoot()
{
    if ( _level == 0) {
        Error::ThrowError("Invalid Syntax : (Invalid Scoop)");
    };
	CkeckDuplication( _used["RootServer"], _used["RootLocation"], "Invalid Syntax : (Duplication In Root Path)");
    _idx++;

    if ( IsSeparator() )
        Error::ThrowError("Invalid Syntax : (Root Has Invalid Path)");
    _idx++;
    if (  _data[_idx] == ";" )
        _idx++;
}

void    Validation::IsValidIndex()
{
    if ( _level == 0 ) {
        Error::ThrowError("Invalid Syntax : (Invalid Scoop)");
    }

	CkeckDuplication(_used["IndexServer"], _used["IndexLocation"], "Invalid Syntax : (Duplication In Index File)");
    _idx++;

    if ( _data[_idx] == ";" )
        Error::ThrowError("Invalid Syntax : (Index Must Have a Name)");
    while ( _idx < (int)_data.size() && !IsSeparator() )
    {
        if ( _data[_idx].find("/") != std::string::npos)
            Error::ThrowError("Invalid Syntax : (Index Is An Absolute Path)");
        _idx++;
    }
    if (  _data[_idx] == ";" )
        _idx++;

}

void    Validation::ScopValidation()
{    
    while( _idx < (int)_data.size() )
    {
        if (  _data[this->_idx] == "}" )
        {
            if ( _level == 0 )
                Error::ThrowError("Invalid Syntax");
            else
            {
                _level--;
                _idx++;
                return ;
            }
        }
        Map::iterator it = _map.find(_data[_idx]);
        if ( it == _map.end() )
            Error::ThrowError("Invalid Syntax");
        (this->*it->second)();
    }
    if ( _level != 0 )
        Error::ThrowError("Invalid Syntax");

}

void    Validation::CheckValidation()
{
    CreateMap();
    ScopValidation();
}
