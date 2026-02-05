/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oel-bann <oel-bann@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 20:05:46 by oel-bann          #+#    #+#             */
/*   Updated: 2026/02/05 22:29:18 by oel-bann         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"


//GET /index.php HTTP/1.1
//Host: localhost:8080
//Connection: keep-alive
//Cache-Control: max-age=0
//sec-ch-ua: "Chromium";v="142", "Google Chrome";v="142", "Not_A Brand";v="99"
//sec-ch-ua-mobile: ?0
//sec-ch-ua-platform: "Linux"
//Upgrade-Insecure-Requests: 1
//User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/142.0.0.0 Safari/537.36
//Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
//Sec-Fetch-Site: none
//Sec-Fetch-Mode: navigate
//Sec-Fetch-User: ?1
//Sec-Fetch-Dest: document
//Accept-Encoding: gzip, deflate, br, zstd
//Accept-Language: fr-FR,fr;q=0.9,en-US;q=0.8,en;q=0.7

map<string, string> Request::_reqDirectives = 
{
    {"Content-Type"   ,     "CONTENT_TYPE"},
    {"Content-Length" ,     "CONTENT_LENGTH"},
    {"Host"           ,     "HTTP_HOST"},
    {"User-Agent"     ,     "HTTP_USER_AGENT"},
    {"Accept"         ,     "HTTP_ACCEPT"},
    {"Accept-Language",     "HTTP_ACCEPT_LANGUAGE"},
    {"Accept-Encoding",     "HTTP_ACCEPT_ENCODING"},
    {"Connection"     ,     "HTTP_CONNECTION"},
    {"Cookie"         ,     "HTTP_COOKIE"},
    {"Referer"        ,     "HTTP_REFERER"},
    {"Origin"         ,     "HTTP_ORIGIN"}
};

Request::Request()
{
    _fist_buff = true;
}

bool Request::parsPath(string path)
{
    size_t pos = path.find('?', pos);

    if (path[0] != '/')
        return(false);
    _env["REQUEST_URI"] = path;
    if (pos != string::npos)
    {
        _env["SCRIPT_NAME"] = path.substr(0, pos);
        _env["QUERY_STRING"] = path.substr(pos + 1);
    }
    else
    {
        _env["SCRIPT_NAME"] = path;
        _env["QUERY_STRING"] = "";
    } 
    return(true);
}
/*
 char* token = strtok(str, "[");
 token = strtok(str, "]");
*/
void Request::parsHost(string Host)
{
    size_t pos = 0;

    if (Host.find(':') != string::npos)
    {
        _env["SERVER_NAME"] = Host.substr(0, Host.find(':'));
        _env["SERVER_PORT"] = Host.substr(Host.find(':') + 1);
        if ()
    }
}

void Request::parsHttpStandard(string httpStandard)
{
    stringstream ss(httpStandard);
    string method, path, httpv;
    string extra;

    if (!(ss >> method >> path >> httpv) || (ss >> extra))
        Error::ThrowError("The Request HttpStandard is Invalid");
    if (_Handlers.find(method) == _Handlers.end())
        Error::ThrowError("The Request method Not Found");
    if (!(httpv == "HTTP/1.1" || httpv == "HTTP/1.0"))
        Error::ThrowError("The Request Protocol version Invalid");
    if (parsPath(path))
        Error::ThrowError("The Request Path Invalid");
    _env["REQUEST_METHOD"] = method;
    _env["SERVER_PROTOCOL"] = httpv;
}

void Request::ParseHeader(istringstream iss)
{
    string line;
    string key;
    string value;

    if (getline(iss, line))
        parsHttpStandard(line);
    else
        Error::ThrowError("Empty Request");
    while (getline(iss, line) && !line.empty())
    {
        if (line.find(':') != string::npos)
        {
            key = line.substr(0, line.find(':'));
            value = line.substr(line.find(':') + 1);
        }
        if (_reqDirectives.find(key) != _reqDirectives.end())
            _env[_reqDirectives[key]] = value;
        if (key == "Host")
            parsHost(value);
    }
    
    line.str()
}

// void Request::handleGet(istringstream iss)
// {
    
// }

void Request::handlePost(std::string)
{
    
}

// void Request::handleDelete(istringstream iss)
// {
    
// }



void Request::ParseRequest(string request_buff)
{
    if (_fist_buff)
    {
        istringstream iss(request_buff);
        _fist_buff = false;
    }

    for (size_t i = 0; request[i]; i++)
    {
        string line = r
    }
    
    
}

bool Request::isComplete(char *request)
{
    this->request += request;
	ParseRequest();
	return false;
}

int main()
{
    Request req;
    req.AddBuffer(NULL);
    return 0;
}
