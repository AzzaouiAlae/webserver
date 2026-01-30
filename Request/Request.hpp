/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oel-bann <oel-bann@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 20:05:48 by oel-bann          #+#    #+#             */
/*   Updated: 2026/01/27 20:09:38 by oel-bann         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Headers.hpp"
class Request
{
private:
    Request();
    std::string body;
public:
    Request(std::string request);
};

// /n/r