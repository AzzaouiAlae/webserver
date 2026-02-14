/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oel-bann <oel-bann@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/14 01:39:07 by oel-bann          #+#    #+#             */
/*   Updated: 2026/02/14 02:15:19 by oel-bann         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cgi.hpp"

void Cgi::run_cgi(int infd, int outfd, Request &req)
{
    pid_t pid = fork();

    if (pid == -1)
        Error::ThrowError("Fork Failed");
    else if (!pid)
    {
        Environment::CreateEnv(req.getrequestenv());
        dup2(0, infd);
        close(infd);
        dup2(1, outfd);
        close(outfd);
        execve(get_);
    }
    
}
