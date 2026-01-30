/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oel-bann <oel-bann@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 20:05:46 by oel-bann          #+#    #+#             */
/*   Updated: 2026/01/30 19:50:50 by oel-bann         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"


//GET / HTTP/1.1
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

void Request::parsHttpStandard(string httpStandard)
{
    stringstream ss(httpStandard);
    string method, path, httpv;
    string extra;

    if (!(ss >> method >> path >> httpv) || (ss >> extra))
        Error::ThrowError("The HttpStandard is Invalid");
    if (_Handlers.find(method) == _Handlers.end())
        Error::ThrowError("The method Not Found");
    if (httpv)
}
void Request::ParseHeader(istringstream iss)
{
    string line;

    if (getline(iss, line))
        parsHttpStandard(line);
    else
        Error::ThrowError("Empty Request");
    while (getline(iss, line) && !line.empty())
    {
        if (getline(iss, line))
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
