#include "Tokenizing.hpp"

Tokenizing::Tokenizing(string filepath): _filepath(filepath)
{
    openConfFile();
}

void Tokenizing::openConfFile()
{
	DEBUG("Tokenizing") << "Attempting to open file stream for: " << _filepath;

    _file.open(_filepath.c_str());
    if (!_file.is_open())
    {
        Error::ThrowError("The ConfigFile Can't Open");
    }

	DEBUG("Tokenizing") << "File stream opened successfully.";
}

const vector<string>& Tokenizing::get_tokens() const
{
    return (_tokens);
}

char Tokenizing::shearch_delimiter(string& str, string delimiters)
{   
    size_t pos = 0;
    size_t spos = str.find(delimiters[0]);
    char delimiter = '\0';
    for (size_t i = 0; i < delimiters.size(); i++)
    {
        pos = str.find(delimiters[i]);
        if (pos != string::npos && pos <= spos)
        {
            spos = pos;
            delimiter = str[spos];
        }
    }
    return (delimiter);
}
void Tokenizing::split_tokens()
{
	INFO() << "Starting configuration tokenization for file: " << _filepath;
    char c;
    string token = "";

    // Read the file character by character to control parsing manually
    while (_file.get(c))
    {
        // 1. Handle Quotes (Start of string)
        if (c == '\"' || c == '\'')
        {
			
            // If we were building a token (e.g., key="value"), push "key" first.
            if (!token.empty())
            {
				DDEBUG("Tokenizing") << "  -> Generated Token (pre-quote): '" << token << "'";
                _tokens.push_back(token);
                token.clear();
            }

            char quoteType = c; // Remember if it was ' or "

            // Loop until we find the closing quote
            while (_file.get(c) && c != quoteType)
            {
                token += c;
            }

			DDEBUG("Tokenizing") << "  -> Generated Token (quoted): '" << token << "'";
            // We hit the closing quote or EOF. 
            // Push the content inside the quotes as a single token.
            _tokens.push_back(token);
            token.clear();
        }
        // 2. Handle Special Delimiters (; { })
        else if (c == ';' || c == '{' || c == '}')
        {
            // If we had a word before the delimiter, push it
            if (!token.empty())
            {
				DDEBUG("Tokenizing") << "  -> Generated Token (word): '" << token << "'";
                _tokens.push_back(token);
                token.clear();
            }
            // Push the delimiter as its own token
            string delim(1, c);
			DDEBUG("Tokenizing") << "  -> Generated Token (delimiter): '" << delim << "'";
            _tokens.push_back(delim);
        }
        // 3. Handle Whitespace (Space, Tab, Newline)
        else if (isspace(c))
        {
            // Whitespace marks the end of a token
            if (!token.empty())
            {
				DDEBUG("Tokenizing") << "  -> Generated Token (word): '" << token << "'";
                _tokens.push_back(token);
                token.clear();
            }
        }
        // 4. Regular Characters
        else
        {
            token += c;
        }
    }

    // Push any remaining token at the end of the file
    if (!token.empty())
    {
		DDEBUG("Tokenizing") << "  -> Generated Token (EOF): '" << token << "'";
        _tokens.push_back(token);
    }

	INFO() << "Tokenization complete. Total tokens generated: " << _tokens.size();
}
