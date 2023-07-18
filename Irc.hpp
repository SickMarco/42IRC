/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Irc.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/12 13:52:11 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/18 18:45:01 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IRC_HPP
#define IRC_HPP

#include <iostream>
#include <cstdlib>

std::string trimMessage(const char* buffer, size_t startIndex);
std::string removeCRLF(const char* buffer);
void 		printStringNoP(const char* str, std::size_t length);

#endif