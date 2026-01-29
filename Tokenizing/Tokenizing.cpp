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

char Tokenizing::shearch_delimiter(std::string& str, std::string delimiters)
{   
    size_t pos = 0;
    size_t spos = str.find(delimiters[0]);
    char delimiter = '\0';
    for (size_t i = 0; i < delimiters.size(); i++)
    {
        pos = str.find(delimiters[i]);
        if (pos != std::string::npos && pos <= spos)
        {
            spos = pos;
            delimiter = str[spos];
        }
    }
    return (delimiter);
}

void Tokenizing::split_tokens()
{
    std::string word;
    char c = '\0';
    while (_file >> word)
    {
        while ((c = shearch_delimiter(word, ":;{}")) && c != '\0')
        {
            size_t pos = word.find(c);
            if (pos > 0)
                _tokens.push_back(word.substr(0, pos));
            _tokens.push_back(word.substr(pos, 1));
            word.erase(0, pos + 1);
        }
        if (word.size() > 0)
            _tokens.push_back(word);
    }
}

