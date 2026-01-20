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
    std::string word;
    std::string line;
    
    while (_file >> word)
    {
        _tokens.push_back(word);
        if (word == "listen")
        {
            if (std::getline(_file, line, ':'))
                _tokens.push_back(line);
            if (std::getline(_file, line, ';'))
                _tokens.push_back(line);
        }
    }
    
}

// int main(int argc, char const *argv[])
// {
//     Tokenizing con("Tokenizing/Config");

//     con.split_tokens();
//     for (std::vector<std::string>::const_iterator it = con.get_tokens().begin(); it != con.get_tokens().end(); ++it)
//         std::cout << *it;
//     return 0;
// }
