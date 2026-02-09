/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Environment.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oel-bann <oel-bann@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/22 23:36:42 by oel-bann          #+#    #+#             */
/*   Updated: 2026/01/27 20:03:27 by oel-bann         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Environment.hpp"

void Environment::AddEnvPair(std::pair<std::string, std::string> pair)
{
    setenv(pair.first.c_str(), pair.second.c_str(), 1);
}

void Environment::CreateEnv(std::map<std::string, std::string> env)
{
    std::for_each(env.begin(), env.end(), AddEnvPair);
}
