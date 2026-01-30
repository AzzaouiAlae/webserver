/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oel-bann <oel-bann@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 20:05:46 by oel-bann          #+#    #+#             */
/*   Updated: 2026/01/30 18:39:07 by oel-bann         ###   ########.fr       */
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
    {"Content-Type",   "CONTENT_TYPE"},
    {"Content-Length", "CONTENT_LENGTH"},
};

Request::Request()
{
    _fist_buff = true;
}


void Request::fill_directives()
{
    _reqDirectives
}

void Request::ParseHeader(istringstream iss)
{
    
    istringstream lineStream;
    string line;
    while (getline(iss, line))
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
        ParseTheFirst2Line()
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
