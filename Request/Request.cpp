/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oel-bann <oel-bann@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 20:05:46 by oel-bann          #+#    #+#             */
/*   Updated: 2026/02/08 00:02:14 by oel-bann         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

// GET /index.php HTTP/1.1
// Host: localhost:8080
// Connection: keep-alive
// Cache-Control: max-age=0
// sec-ch-ua: "Chromium";v="142", "Google Chrome";v="142", "Not_A Brand";v="99"
// sec-ch-ua-mobile: ?0
// sec-ch-ua-platform: "Linux"
// Upgrade-Insecure-Requests: 1
// User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/142.0.0.0 Safari/537.36
// Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
// Sec-Fetch-Site: none
// Sec-Fetch-Mode: navigate
// Sec-Fetch-User: ?1
// Sec-Fetch-Dest: document
// Accept-Encoding: gzip, deflate, br, zstd
// Accept-Language: fr-FR,fr;q=0.9,en-US;q=0.8,en;q=0.7

// map<string, string> Request::_reqDirectives;

// void Request::initReqDirectives()
// {
//     if (_reqDirectives.size() != 0)
//         return;
//     _reqDirectives["Content-Type"]        = "CONTENT_TYPE";
//     _reqDirectives["Content-Length"]      = "CONTENT_LENGTH";

//     _reqDirectives["Host"]                = "HTTP_HOST";
//     _reqDirectives["User-Agent"]          = "HTTP_USER_AGENT";
//     _reqDirectives["Accept"]              = "HTTP_ACCEPT";
//     _reqDirectives["Accept-Language"]     = "HTTP_ACCEPT_LANGUAGE";
//     _reqDirectives["Accept-Encoding"]     = "HTTP_ACCEPT_ENCODING";
//     _reqDirectives["Connection"]          = "HTTP_CONNECTION";
//     _reqDirectives["Cookie"]              = "HTTP_COOKIE";
//     _reqDirectives["Referer"]             = "HTTP_REFERER";
//     _reqDirectives["Origin"]              = "HTTP_ORIGIN";
// }


// Request::Request()
// {
//     initReqDirectives();
    
//     _headerparsed = false;
//     _pos = 0;
//     _headerbuff = "";
//     _env["SERVER_SOFTWARE"] = "webserv/1.0";
//     _env["GATEWAY_INTERFACE"] = "CGI/1.1";
// }

bool Request::getFullLine (string &line)
{
    int index = 0;
    line.clear();

    while (_headerbuff[index] && _headerbuff[index] != '\n')
    {
        if (index > MAXHEADERSIZE)
            Error::ThrowError("The Header is Bigger Than Expected");
        line += _headerbuff[index];
        index++;
    }
    if (_headerbuff[index] == '\n')
    {
        _headerbuff.erase(0, index);
        line += "\n";
    }
    else
        return (false);
    return true;
}

// bool Request::parsPath(string path)
// {
//     size_t pos = path.find('?');

//     if (path[0] != '/')
//         return(false);
//     _env["REQUEST_URI"] = path;
//     if (pos != string::npos)
//     {
//         _env["SCRIPT_NAME"] = path.substr(0, pos);
//         _env["QUERY_STRING"] = path.substr(pos + 1);
//     }
//     else
//     {
//         _env["SCRIPT_NAME"] = path;
//         _env["QUERY_STRING"] = "";
//     } 
//     return(true);int argc, char const *argv[]eader()
// {
//     string line;
//     string key;
//     string value;

//     if (getFullLine(line))
//         parsHttpStandard(line);
//     else
//         Error::ThrowError("Empty Request");
//     while (getline(iss, line) && !line.empty())
//     {
//         if (line.find(':') != string::npos)
//         {
//             key = line.substr(0, line.find(':'));
//             value = line.substr(line.find(':') + 1);
//         }
//         if (_reqDirectives.find(key) != _reqDirectives.end())
//             _env[_reqDirectives[key]] = value;
//         else
//             _env[key] = value;
//         if (key == "Host")
//             Validation::parseListen(value, _env["SERVER_NAME"], _env["SERVER_PORT"]);
//     }
//     getline(iss, _env["Body"], '\0');
//     for (map<string, string>::iterator it = _env.begin(); it != _env.end(); ++it)
//     {
//         cout << "[" << it->first << "]" << " = " << it->second << std::endl;
//     }
// }

// // void Request::handleGet(istringstream iss)
// // {
    
// // }

// void Request::handlePost(std::string)
// {
    
// }

// // void Request::handleDelete(istringstream iss)
// // {
    
// // }



// bool Request::ParseRequest(string request_buff)
// {
//     if (!_headerparsed)
//     {
//         _headerbuff += request_buff;
//         if (!ParseHeader())
//             return (false);
//     }
//     // ParseHeader(iss);
//     // for (size_t i = 0; request[i]; i++)
//     // {
//     //     string line = r
    
// }
    
    
// }

// bool Request::isComplete(char *request)
// {
//     this->request += request;
// 	ParseRequest();
// 	return false;
// }

// int main()
// {
//     Request req;
//     req.AddBuffer(NULL);
//     return 0;
// }


