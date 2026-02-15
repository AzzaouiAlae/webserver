/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oel-bann <oel-bann@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/14 01:39:07 by oel-bann          #+#    #+#             */
/*   Updated: 2026/02/15 01:06:49 by oel-bann         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cgi.hpp"

bool Cgi::isExeted()
{
    int status;
    if (waitpid(_pid, &status, WNOHANG) != _pid)
        return false;
    return true;
}


void resetTime()
{
    
}

long getTime()
{
    
}

void Cgi::run_cgi(int infd, int outfd, Request &req, char** exec)
{
    _pid = fork();


    if (_pid == -1)
        Error::ThrowError("Fork Failed");
    else if (!_pid)
    {
        Environment::CreateEnv(req.getrequestenv());
        dup2(0, infd);
        close(infd);
        dup2(1, outfd);
        close(outfd);
        execve(exec[0], exec, environ);
        exit(1);
    }
}
