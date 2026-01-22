/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Environment.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oel-bann <oel-bann@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/22 23:36:42 by oel-bann          #+#    #+#             */
/*   Updated: 2026/01/22 23:57:25 by oel-bann         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Environment.hpp"

void Environment::FillEnv(char **env)
{
    for (size_t i = 0; env[i]; i++)
    {
        std::string str(env[i]);
        std::vector<std::string> v;
        size_t pos = 0;
        str.find('=', pos);
        if (pos != std::string::npos)
            v.push_back(str.substr(pos + 1));
        _env.insert(std::make_pair(str.substr(0, str.find('=', pos)), v));
    }
}

void Environment::AddConfigFile(void)
{
    
}

std::vector<std::string>& Environment::GetEnvValue(std::string key)
{
    
}
