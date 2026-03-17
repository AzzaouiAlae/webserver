/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Environment.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aazzaoui <aazzaoui@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/22 23:36:42 by oel-bann          #+#    #+#             */
/*   Updated: 2026/02/20 18:16:05 by aazzaoui         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Environment.hpp"

set<string> Environment::specialKeys;

void Environment::initSpecialKeys()
{
    specialKeys.insert("CONTENT_TYPE");
    specialKeys.insert("CONTENT_LENGTH");
    specialKeys.insert("QUERY_STRING");
    specialKeys.insert("REQUEST_METHOD");
    specialKeys.insert("SCRIPT_NAME");
    specialKeys.insert("SERVER_NAME");
    specialKeys.insert("SERVER_PORT");
    specialKeys.insert("SERVER_PROTOCOL");
    specialKeys.insert("REQUEST_URI");

    specialKeys.insert("AUTH_TYPE");
    specialKeys.insert("GATEWAY_INTERFACE");
    specialKeys.insert("PATH_INFO");
    specialKeys.insert("PATH_TRANSLATED");
    specialKeys.insert("REMOTE_ADDR");
    specialKeys.insert("REMOTE_HOST");
    specialKeys.insert("REMOTE_IDENT");
    specialKeys.insert("REMOTE_USER");
    specialKeys.insert("SERVER_SOFTWARE");

    specialKeys.insert("SCRIPT_FILENAME");
    specialKeys.insert("DOCUMENT_ROOT");     
    specialKeys.insert("SERVER_ADDR");       
    specialKeys.insert("REDIRECT_STATUS");
}

string Environment::formatCgiEnvKey(const string& key) 
{
    if (specialKeys.empty()) {
        initSpecialKeys();
    }
    
    string formattedKey = "";

    for (size_t i = 0; i < key.length(); ++i) {
        if (key[i] == '-') {
            formattedKey += '_';
        } else {
            formattedKey += toupper(key[i]);
        }
    }

    if (specialKeys.find(formattedKey) != specialKeys.end()) {
        return formattedKey;
    }

    if (formattedKey.substr(0, 5) != "HTTP_") {
        formattedKey = "HTTP_" + formattedKey;
    }

    return formattedKey;
}

void Environment::AddEnvPair(pair<string, string> pair)
{
    string envKey = formatCgiEnvKey(pair.first);
    setenv(envKey.c_str(), pair.second.c_str(), 1);
    DDEBUG("Environment")
        << "Set environment variable: " 
        << envKey << "=" << pair.second;
}

void Environment::CreateEnv(map<string, string> env)
{
    for_each(env.begin(), env.end(), AddEnvPair);
}
