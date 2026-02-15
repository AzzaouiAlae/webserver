/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oel-bann <oel-bann@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/14 01:37:26 by oel-bann          #+#    #+#             */
/*   Updated: 2026/02/15 02:55:26 by oel-bann         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../Headers.hpp"

class Cgi
{
    long _time;
    pid_t _pid;
public:
    void run_cgi(int infd, int outfd, Request &req, char** exec);
    bool isExeted();
    void resetTime();
    long getTime();
    
};
