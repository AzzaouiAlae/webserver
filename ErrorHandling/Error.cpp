/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Error.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aazzaoui <aazzaoui@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/20 19:14:05 by oel-bann          #+#    #+#             */
/*   Updated: 2026/02/11 11:22:14 by aazzaoui         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Error.hpp"

std::string Error::_REDCOLOR     = "\x1b[31m";
std::string Error::_GREENCOLOR   = "\x1b[32m";
std::string Error::_YELLOWCOLOR  = "\x1b[33m";
std::string Error::_BLUECOLOR    = "\x1b[34m";
std::string Error::_MAGENTACOLOR = "\x1b[35m";
std::string Error::_CYANCOLOR    = "\x1b[36m";
std::string Error::_BOLD         = "\x1b[1m";
std::string Error::_RESETCOLOR   = "\x1b[0m";

void Error::printError(std::string custoMessage)
{
    cerr << _REDCOLOR  << _BOLD <<  "Error :\n	" << custoMessage << _RESETCOLOR << std::endl;
}

void Error::ThrowError(std::string custoMessage)
{
    throw std::runtime_error(custoMessage);
}

ErrorType Error::errorType;

