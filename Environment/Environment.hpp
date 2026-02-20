/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Environment.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aazzaoui <aazzaoui@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/22 23:25:36 by oel-bann          #+#    #+#             */
/*   Updated: 2026/02/20 18:16:05 by aazzaoui         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../Headers.hpp"

class Environment
{
    Environment();
    static void AddEnvPair(pair<string, string> pair);
public:
    static void CreateEnv(map<string, string> env);
};
