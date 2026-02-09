/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Environment.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oel-bann <oel-bann@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/22 23:25:36 by oel-bann          #+#    #+#             */
/*   Updated: 2026/02/05 23:03:09 by oel-bann         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../Headers.hpp"

class Environment
{
    Environment();
    static void AddEnvPair(std::pair<std::string, std::string> pair);
public:
    void CreateEnv(std::map<std::string, std::string> env);
};
