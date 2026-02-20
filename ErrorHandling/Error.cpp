/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Error.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aazzaoui <aazzaoui@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/20 19:14:05 by oel-bann          #+#    #+#             */
/*   Updated: 2026/02/20 18:16:05 by aazzaoui         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Error.hpp"

string Error::_REDCOLOR     = "\x1b[31m";
string Error::_GREENCOLOR   = "\x1b[32m";
string Error::_YELLOWCOLOR  = "\x1b[33m";
string Error::_BLUECOLOR    = "\x1b[34m";
string Error::_MAGENTACOLOR = "\x1b[35m";
string Error::_CYANCOLOR    = "\x1b[36m";
string Error::_BOLD         = "\x1b[1m";
string Error::_RESETCOLOR   = "\x1b[0m";

void Error::printError(string custoMessage)
{
    cerr << _REDCOLOR  << _BOLD <<  "Error :\n	" << custoMessage << _RESETCOLOR << endl;
}

void Error::ThrowError(string custoMessage)
{
    throw runtime_error(custoMessage);
}

ErrorType Error::errorType;

