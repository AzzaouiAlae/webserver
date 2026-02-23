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

void Environment::AddEnvPair(pair<string, string> pair)
{
    setenv(pair.first.c_str(), pair.second.c_str(), 1);
}

void Environment::CreateEnv(map<string, string> env)
{
    for_each(env.begin(), env.end(), AddEnvPair);
}
