/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Error.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aazzaoui <aazzaoui@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/20 19:11:20 by oel-bann          #+#    #+#             */
/*   Updated: 2026/02/20 18:16:05 by aazzaoui         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../Headers.hpp"

enum ErrorType
{
    non,
    NotFound,
    BadRequest,
    Forbidden,
    InternalServerError,

};

class Error
{
private:
    static string _REDCOLOR    ;
    static string _GREENCOLOR  ;
    static string _YELLOWCOLOR ;
    static string _BLUECOLOR   ;
    static string _MAGENTACOLOR;
    static string _CYANCOLOR   ;
    static string _BOLD        ;
    static string _RESETCOLOR  ;

    Error();
public:
    static ErrorType errorType;
    static void printError(string custoMessage);
    static void ThrowError(string custoMessage);

};
