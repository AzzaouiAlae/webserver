#include "../Headers.hpp"

Validation::Validation(std::vector<std::string> inputData)
    :  _idx(0), _duplicateServer_Root(false), _duplicateServer_Index(false),
    _duplicateLocation_Root(false), _duplicateLocation_Index(false),
    _level(0), _data(inputData) {}

void    Validation::CreateMap()
{
    _map["server"] = &Validation::IsValidServer;
    _map["listen"] = &Validation::IsValidListen;
    _map["server_name"] = &Validation::IsValidServerName;
    _map["root"] = &Validation::IsValidRoot;
    _map["index"] = &Validation::IsValidIndex;
    _map["location"] = &Validation::IsValidLocation;
    
}

void    Validation::ResetServerSeting()
{
    _duplicateServer_Index = false;
    _duplicateServer_Root = false;

}
void    Validation::ResetLocationSeting()
{
    _duplicateLocation_Index = false;
    _duplicateLocation_Root = false;

}


void    Validation::IsValidServer()
{
    //std::cout << _data[_idx] << "    \n" ;
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
    //std::cout << _data[_idx] << "    \n" ;
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
    //std::cout << _data[_idx] << "    \nport =="  << port;
    
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
    //std::cout << _data[_idx] << "    \n" ;
    _idx++;
    PortOnly();
}

void    Validation::IsValidListen()
{
    if ( _level == 2)
        Error::ThrowError("Invalid Syntax : (Invslid Scoop)");
    //std::cout << _data[_idx] << "    \n" ;
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
    if ( _level == 2)
        Error::ThrowError("Invalid Syntax : (Invslid Scoop)");
    //std::cout << _data[_idx] << "    \n" ;
    _idx++;

    if ( _data[_idx] == ";" )
        Error::ThrowError("Invalid Syntax : (server_name Must Have a Name)");
    while ( _idx < (int)_data.size() && !IsSeparator() )
        _idx++;
    if (  _data[_idx] == ";" )
        _idx++;
}

void    Validation::CkeckDuplicationRoot(std::string msg)
{
    if ( _level == 1)
    {
        if ( _duplicateServer_Root == true )
            Error::ThrowError(msg);
        else
            _duplicateServer_Root = true;
        return ;
    }
    else if (_level == 2)
    {
        if ( _duplicateLocation_Root == true )
            Error::ThrowError(msg);
        else
            _duplicateLocation_Root = true;
        return ;
    }
    else
        Error::ThrowError("Invalid Syntax : (Invslid Scoop)");
}

void    Validation::IsValidRoot()
{
	CkeckDuplicationRoot("Invalid Syntax : (Duplication In Root Path)");
    //std::cout << _data[_idx] << "    \n" ;
    _idx++;

    if ( IsSeparator() )
        Error::ThrowError("Invalid Syntax : (Root Has Invalid Path)");
    _idx++;
    if (  _data[_idx] == ";" )
        _idx++;
}
void    Validation::CkeckDuplicationIndex(std::string msg)
{
    if ( _level == 1)
    {
        if ( _duplicateServer_Index == true )
            Error::ThrowError(msg);
        else
            _duplicateServer_Index = true;
        return ;
    }
    else if (_level == 2)
    {
        if ( _duplicateLocation_Index == true )
            Error::ThrowError(msg);
        else
            _duplicateLocation_Index = true;
        return ;
    }
    else
        Error::ThrowError("Invalid Syntax : (Invslid Scoop)");
}

void    Validation::IsValidIndex()
{	
	CkeckDuplicationIndex("Invalid Syntax : (Duplication In Index File)");
    
    //std::cout << _data[_idx] << "    \n" ;
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
        if ( it == _map.end() || ( it->first != "server" && _level == 0 ) )
            Error::ThrowError("Invalid Syntax");
        (this->*it->second)();
    }
    if ( _level != 0)
        Error::ThrowError("Invalid Syntax");

}
