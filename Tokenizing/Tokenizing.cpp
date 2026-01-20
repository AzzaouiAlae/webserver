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

void Tokenizing::split_tokens()
{
	std::string line;
    while (std::getline(std::cin, line))
    {
        /* code */
    }
    
}