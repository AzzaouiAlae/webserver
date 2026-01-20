/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Error.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oel-bann <oel-bann@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/20 19:11:20 by oel-bann          #+#    #+#             */
/*   Updated: 2026/01/20 19:25:26 by oel-bann         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>

class Error
{
private:
    static std::string _REDCOLOR    ;
    static std::string _GREENCOLOR  ;
    static std::string _YELLOWCOLOR ;
    static std::string _BLUECOLOR   ;
    static std::string _MAGENTACOLOR;
    static std::string _CYANCOLOR   ;
    static std::string _BOLD        ;
    static std::string _RESETCOLOR  ;
    Error();
public:
    static void printError(std::string custoMessage);
};
