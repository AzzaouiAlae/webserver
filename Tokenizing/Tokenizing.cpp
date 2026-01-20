#include "Tokenizing.hpp"

Tokenizing::Tokenizing(std::string filepath): _filepath(filepath)
{
    openConfFile();
}

void Tokenizing::openConfFile()
{
    _file.open(_filepath);
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

void Tokenizing::split_tokens()
{
    while (std::getline(std::cin))
    {
        /* code */
    }
    
}