#include "Tokenizing.hpp"

Tokenizing::Tokenizing(std::string filepath): _filepath(filepath)
{
    openConfFile();
}

void Tokenizing::openConfFile()
{
    _file.open(_filepath.c_str());
    if (!_file.is_open())
    {
        Error::printError("The ConfigFile Can't Open");
        exit(1);
    }
}

const std::vector<std::string>& Tokenizing::get_tokens() const
{
    return (_tokens);
}


void Tokenizing::trim(std::string &str)
{
    size_t i  = 0;
    while (i < str.size() && (str[i] == ' '  || (str[i] >= 9 && str[i] <= 13)))
        i++;
    str.erase(0,i);
    
    i = str.size();
    while (i > 0 && (str[i - 1] == ' '  || (str[i - 1] >= 9 && str[i - 1] <= 13)))
        i--;
    
    str.erase(i);
}

void Tokenizing::split_tokens()
{
    std::string word;
    std::string line;
    
    while (_file >> word)
    {
        _tokens.push_back(word);
        if (word == "listen")
        {
            if (std::getline(_file, line))
            {
                trim(line);
                size_t pos = line.find(":");
                if (pos != std::string::npos)
                {
                    _tokens.push_back(std::substr(0, pos));
                    _t
                }
            }
                _tokens.push_back(line);
            if (std::getline(_file, line, ';'))
                _tokens.push_back(line);
            // std::ltr
            //     line.substr(std::)
        }
    }
    std::string str("          oussa ama kdkkd djjd            ");
    trim(str);
    std::cout << "|" << str << "|" << std::endl;
}

