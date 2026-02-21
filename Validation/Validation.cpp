#include "Validation.hpp"
#include "../Socket/Socket.hpp"

std::vector<std::string> Validation::_skiped;

void Validation::CreateMimeMap()
{
	std::map<std::string, std::string> &mime = Singleton::GetMime();
	bool ismaintype = false;
	string maintype = "";

	while (++_idx < (int)_data.size() && _data[_idx] != "}")
	{
		ismaintype = true;
		maintype = "";
		for (; _data[_idx] != ";"; _idx++)
		{
			if (!ismaintype)
				mime[_data[_idx]] = maintype;
			else if (ismaintype && _data[_idx].find('/') == string::npos)
				Error::ThrowError("Invalid Syntax");
			else
			{
				maintype = _data[_idx];
				ismaintype = false;
			}
		}
	}

	// for (map<string, string>::iterator it = mime.begin(); it != mime.end(); it++)
	// {
	// 	cout << "|" << it->first << ":" << it->second << "|" << endl;
	// }
}

void Validation::IsValidTypes()
{
	if (_level != 0 || _data[++_idx] != "{")
		Error::ThrowError("Invalid Syntax");
	else
		_level++;
	CreateMimeMap();
	CheckValidation();
}

void Validation::AddDirective(std::string value)
{
	if (_level == 1)
		Parsing::AddDirective(*(Parsing::currentServer), value);
	else if (_level == 2)
		Parsing::AddDirective(*(Parsing::currentLocation), value);
}

Validation::Validation(std::vector<std::string> inputData)
	: _idx(0), _level(0), _data(inputData)
{
	CreateMap();
	CreateServerdMap();
	CreateLocationMap();
	CreateSkipedData();
}

void Validation::CreateLocationMap()
{
	_useLocation["Location"] = false;
	_useLocation["Root"] = false;
	_useLocation["Index"] = false;
	_useLocation["Autoindex"] = false;
	_useLocation["Return"] = false;
	_useLocation["ClientMaxBodySize"] = false;
	_useLocation["AllowMethods"] = false;
}

void Validation::CreateServerdMap()
{
	_useServer["Server"] = false;
	_useServer["Listen"] = false;
	_useServer["Server_name"] = false;
	_useServer["Root"] = false;
	_useServer["Index"] = false;
	_useServer["Autoindex"] = false;
	_useServer["Return"] = false;
	_useServer["ClientMaxBodySize"] = false;
	_useServer["AllowMethods"] = false;
}

void Validation::CreateSkipedData()
{
	if (_skiped.empty())
	{
		_skiped.push_back("Return");
		_skiped.push_back("Root");
		_skiped.push_back("Index");
		_skiped.push_back("Autoindex");
		_skiped.push_back("Autoindex");
		_skiped.push_back("ClientMaxBodySize");
		_skiped.push_back("AllowMethods");
	}
}

void Validation::CreateMap()
{
	_map["server"] = &Validation::IsValidServer;
	_map["listen"] = &Validation::IsValidListen;
	_map["server_name"] = &Validation::IsValidServerName;
	_map["root"] = &Validation::IsValidRoot;
	_map["index"] = &Validation::IsValidIndex;
	_map["location"] = &Validation::IsValidLocation;
	_map["autoindex"] = &Validation::IsValidAutoindex;
	_map["return"] = &Validation::IsValidReturn;
	_map["error_page"] = &Validation::IsErrorPage;
	_map["client_max_body_size"] = &Validation::IsClientMaxBodySize;
	_map["allow_methods"] = &Validation::IsValidAllowMethods;
	_map["cgi_pass"] = &Validation::IsValidCGIPass;
	_map["types"] = &Validation::IsValidTypes;
}

bool Validation::SkipedOptions(std::string option)
{
	if (std::find(_skiped.begin(), _skiped.end(), option) != _skiped.end())
		return (true);
	return (false);
}

void Validation::CheckExistance(std::pair<std::string, bool> used)
{
	if (!SkipedOptions(used.first) && used.second == false)
		Error::ThrowError("Invalid Syntax : (Missing Element in Server)");
}

void Validation::CheckLevelAndDuplication(bool &first, bool &second, std::string msg)
{
	if (_level == 0)
	{
		Error::ThrowError("Invalid Syntax : (Element out of scope)");
	};
	CkeckDuplication(first, second, "Invalid Syntax : ( Duplication In " + msg + " )");
	_idx++;
}

void Validation::ResetServerSeting()
{
	std::for_each(_useServer.begin(), _useServer.end(), CheckExistance);

	_useServer["Root"] = false;
	_useServer["Index"] = false;
}

void Validation::ResetLocationSeting()
{
	_useLocation["Root"] = false;
	_useLocation["Index"] = false;
}

//      SERVER
void Validation::IsValidServer()
{
	_useServer["Server"] = true;
	_idx++;
	if (_level != 0 || _data[_idx] != "{")
		Error::ThrowError("Invalid Syntax");
	else
		_level++;
	_idx++;
	Parsing::AddServer();
	CheckValidation();
	ResetServerSeting();
}

//      LOCATION
void Validation::IsValidLocation()
{
	_useLocation["Location"] = true;
	_idx++;
	if (_level != 1 || IsSeparator() || _data[_idx][0] != '/')
		Error::ThrowError("Invalid Syntax : (Invalid Location)");
	string &s = _data[_idx];
	_idx++;
	if (_data[_idx] != "{")
		Error::ThrowError("Invalid Syntax : (missing '{' In location )");
	else
		_level++;
	_idx++;
	Parsing::AddLocation(*(Parsing::currentServer));
	Parsing::currentLocation->AddArgument(s);
	CheckValidation();
	ResetLocationSeting();
}

long Validation::ConvertToNumber(std::string num)
{
	char *endptr;
	errno = 0;

	long port = std::strtoll(num.c_str(), &endptr, 10);

	if (num[0] == '+' || errno != 0 || *endptr != '\0')
		Error::ThrowError("Invalid Syntax ( Number Not Valid )");
	return (port);
}

void Validation::PortOnly()
{
	long port = ConvertToNumber(_data[_idx]);

	if (port < 0 || port > 65535)
		Error::ThrowError("Invalid Syntax ( Port Number Out Of Range )");

	Parsing::AddArg(*(Parsing::currentDirective), _data[_idx]);
	_idx++;
}

void Validation::ValidIP()
{
	long Ip = 0;
	size_t countPoint = 0;
	size_t start = 0;
	size_t pos = 0;
	while ((pos = _data[_idx].find('.', start)) != std::string::npos)
	{
		if (countPoint >= 3)
			Error::ThrowError("Invalid Syntax ( IP Address Not Valid )");
		countPoint++;
		Ip = ConvertToNumber(_data[_idx].substr(start, pos - start));

		if (Ip < 0 || Ip > 255)
			Error::ThrowError("Invalid Syntax ( IP Adress: Number Out Of Range )");
		start = pos + 1;
	}
	pos = *(_data[_idx].end() - 1);
	Ip = ConvertToNumber(_data[_idx].substr(start, pos - start));
	if (Ip < 0 || Ip > 255)
		Error::ThrowError("Invalid Syntax ( IP Adress: Number Out Of Range )");
	if (countPoint != 3)
		Error::ThrowError("Invalid Syntax ( IP Address Not Valid )");
	Parsing::AddArg(*(Parsing::currentDirective), _data[_idx]);
	_idx++;
}

void Validation::IpAndPort()
{
	ValidIP();
	_idx++;
	PortOnly();
}

void Validation::parseListen(string str, string &port, string &host)
{
	const char *s1 = strrchr(str.c_str(), ':');
	bool isHost = false;

	if (s1 != NULL)
	{
		host = str.c_str();
		host = host.substr(0, (s1 - str.c_str()));
		port = s1 + 1;
		return;
	}
	for(int i = 0; i < (int)str.length(); i++)
	{
		if(isdigit(str[i]) == false)
		{
			isHost = true;
			break;
		}
	}
	if (isHost) {
		host = str;
		port = "80";
	}
	else
		port = str;
}

//     LISTEN
void    Validation::IsValidListen()
{
    _useServer["Listen"] = true;
    if ( _level != 1)
        Error::ThrowError("Invalid Syntax : (Element out of scope)");
    Validation::AddDirective("listen");
    _idx++;
    if (  _idx + 1 < (int)_data.size() )
    {
		string port, host;
		parseListen(_data[_idx], port, host);
        if (host != "")
		{
			Parsing::AddArg(*(Parsing::currentDirective), host);
			Parsing::AddArg(*(Parsing::currentDirective), port);
		}
        else if ( _data[_idx + 1] == ";"  )
			Parsing::AddArg(*(Parsing::currentDirective), port);
        else
            Error::ThrowError("Invalid Syntax");
        _idx += 2;
    }
    else
        Error::ThrowError("Invalid Syntax");

}

bool Validation::IsSeparator()
{
	if (_data[_idx] == ";" || _data[_idx] == "}" || _data[_idx] == "{")
		return (true);
	return (false);
}

//      SEVERNAME
void Validation::IsValidServerName()
{
	_useServer["Server_name"] = true;
	if (_level != 1)
		Error::ThrowError("Invalid Syntax : (Element out of scope)");
	Validation::AddDirective("server_name");
	_idx++;

	if (_data[_idx] == ";")
		Error::ThrowError("Invalid Syntax : (server_name Must Have a Name)");
	while (_idx < (int)_data.size() && !IsSeparator())
		Parsing::AddArg(*(Parsing::currentDirective), _data[_idx++]);
	if (_data[_idx] == ";")
		_idx++;
}

void Validation::CkeckDuplication(bool &first, bool &second, std::string msg)
{
	if (_level == 1)
	{
		if (first == true)
			Error::ThrowError(msg);
		else
			first = true;
		return;
	}
	else if (_level == 2)
	{
		if (second == true)
			Error::ThrowError(msg);
		else
			second = true;
		return;
	}
	else
		Error::ThrowError("Invalid Syntax : (Element out of scope)");
}

//      ROOT
void Validation::IsValidRoot()
{
	CheckLevelAndDuplication(_useServer["Root"], _useLocation["Root"], "Root");

	Validation::AddDirective("root");
	if (IsSeparator())
		Error::ThrowError("Invalid Syntax : (Root Has Invalid Path)");
	Parsing::AddArg(*(Parsing::currentDirective), _data[_idx]);
	_idx++;
	if (_data[_idx] == ";")
		_idx++;
}

//      INDEX
void Validation::IsValidIndex()
{
	CheckLevelAndDuplication(_useServer["Index"], _useLocation["Index"], "Index");

	Validation::AddDirective("index");
	if (_data[_idx] == ";")
		Error::ThrowError("Invalid Syntax : (Index Must Have a Name)");
	while (_idx < (int)_data.size() && !IsSeparator())
	{
		if (_data[_idx].find("/") != std::string::npos)
			Error::ThrowError("Invalid Syntax : (Index Is An Absolute Path)");

		Parsing::AddArg(*(Parsing::currentDirective), _data[_idx++]);
	}
	if (_data[_idx] == ";")
		_idx++;
}

//      AUTOINDEX
void Validation::IsValidAutoindex()
{
	CheckLevelAndDuplication(_useServer["Autoindex"], _useLocation["Autoindex"], "Autoindex");

	Validation::AddDirective("autoindex");
	if (IsSeparator() || (_data[_idx] != "off" && _data[_idx] != "on"))
		Error::ThrowError("Invalid Syntax : (autoindex Has Invalid option )");
	Parsing::AddArg(*(Parsing::currentDirective), _data[_idx]);
	_idx++;
	if (_data[_idx] == ";")
		_idx++;
}

//      RETURN
void Validation::IsValidReturn()
{
	CheckLevelAndDuplication(_useServer["Return"], _useLocation["Return"], "Return");

	Validation::AddDirective("return");
	long redirectCode = ConvertToNumber(_data[_idx]);
	if (redirectCode != 301 && redirectCode != 302)
		Error::ThrowError("Invalid Syntax : ( Invalid Redirection Code In return )");

	Parsing::AddArg(*(Parsing::currentDirective), _data[_idx]);
	_idx++;

	if (!IsSeparator())
		_idx++;

	if (_data[_idx] == ";")
		_idx++;
}

//    ERRORPAGE
void Validation::IsErrorPage()
{
	bool error_code = false;
	bool error_file = false;

	Validation::AddDirective("error_page");
	_idx++;
	while (_idx < (int)_data.size() && !IsSeparator())
	{
		try
		{
			long errorCode = ConvertToNumber(_data[_idx]);
			if ((errorCode >= 400 && errorCode <= 404) || (errorCode >= 500 && errorCode <= 504))
			{
				Parsing::AddArg(*(Parsing::currentDirective), _data[_idx++]);
				error_code = true;
			}
			else
				break;
		}
		catch (const std::exception &e)
		{
			Parsing::AddArg(*(Parsing::currentDirective), _data[_idx++]);
			error_file = true;
			break;
		}
	}
	if (error_code == false || error_file == false)
		Error::ThrowError("Invalid Syntax : ( Invalid Error Code Or File In Error_page )");
	if (_data[_idx] == ";")
		_idx++;
}

bool Validation::IsByteSizeUnit(std::string &data)
{
	return (strchr("KkMmGg", data[data.size() - 1]));
}

//     CLIENTMAXBODYSIZE
void Validation::IsClientMaxBodySize()
{
	CheckLevelAndDuplication(_useServer["ClientMaxBodySize"], _useLocation["ClientMaxBodySize"], "ClientMaxBodySize");

	Validation::AddDirective("client_max_body_size");

	ConvertToNumber(_data[_idx].substr(0, _data[_idx].size() - 1));
	if (!IsSeparator() && IsByteSizeUnit(_data[_idx]) == true)
		Parsing::AddArg(*(Parsing::currentDirective), _data[_idx++]);
	else
		Error::ThrowError("Invalid Syntax : ( Client_max_body_size Not Valid )");
	if (_data[_idx] == ";")
		_idx++;
}

bool Validation::IsAllowedMethods(std::string &method)
{
	if (method == "GET" || method == "POST" || method == "DELETE")
		return (true);
	return (false);
}

//     ALLOWMETHODS
void Validation::IsValidAllowMethods()
{
	CheckLevelAndDuplication(_useServer["AllowMethods"], _useLocation["AllowMethods"], "AllowMethods");

	Validation::AddDirective("allow_methods");
	if (_data[_idx] == ";")
		_idx++;
	while (_idx < (int)_data.size() && !IsSeparator())
	{
		if (IsAllowedMethods(_data[_idx]) == false)
			Error::ThrowError("Invalid Syntax : ( Invalid Method In allow_methods )");
		Parsing::AddArg(*(Parsing::currentDirective), _data[_idx++]);
	}

	if (_data[_idx] == ";")
		_idx++;
}


void Validation::IsValidCGIPass()
{
	if (_level != 2)
		Error::ThrowError("Invalid Syntax : (Element out of scope)");

	Validation::AddDirective("cgi_pass");
	_idx++;

	if (_data[_idx].find(".") != 0)
		Error::ThrowError("Invalid Syntax : (CgiPass Must Have an extension)");
	Parsing::AddArg(*(Parsing::currentDirective), _data[_idx++]);
	if (!IsSeparator())
		Parsing::AddArg(*(Parsing::currentDirective), _data[_idx++]);
	else
		Error::ThrowError("Invalid Syntax : (CgiPass Must Have a Path)");
	if (_data[_idx] == ";")
		_idx++;
}

void Validation::CheckValidation()
{
	while (_idx < (int)_data.size())
	{
		if (_data[this->_idx] == "}")
		{
			if (_level == 0)
				Error::ThrowError("Invalid Syntax");
			else
			{
				_level--;
				_idx++;
				return;
			}
		}
		Map::iterator it = _map.find(_data[_idx]);
		if (it == _map.end())
			Error::ThrowError("Invalid Syntax");
		(this->*it->second)();
	}
	if (_level != 0)
		Error::ThrowError("Invalid Syntax");
	// if (Singleton::GetServers().size() == 0) {
	// 	Error::ThrowError("Can't bind to any server");
	// }
}
