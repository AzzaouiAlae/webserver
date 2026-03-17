/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Environment.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aazzaoui <aazzaoui@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/22 23:25:36 by oel-bann          #+#    #+#             */
/*   Updated: 2026/03/03 03:55:22 by aazzaoui         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Headers.hpp"

class Environment
{
    Environment();
    static set<string> specialKeys;
    static void initSpecialKeys();
    static void AddEnvPair(pair<string, string> pair);
    static string formatCgiEnvKey(const string& key);
public:
    static void CreateEnv(map<string, string> env);
};
