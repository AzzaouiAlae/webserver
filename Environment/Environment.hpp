/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Environment.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oel-bann <oel-bann@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/22 23:25:36 by oel-bann          #+#    #+#             */
/*   Updated: 2026/01/22 23:57:06 by oel-bann         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../Headers.hpp"

class Environment
{
private:
    static std::map<std::string, std::vector<std::string>> _env;
    Environment();
public:
    void FillEnv(char **env);
    void AddConfigFile(void);
    std::vector<std::string>& GetEnvValue(std::string key);
};
