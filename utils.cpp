/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/15 19:37:31 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/22 12:10:56 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "User.hpp"

std::string trimMessage(const char* buffer, size_t startIndex){
	std::string input = buffer;
	std::string ret;
	size_t endIndex = input.find('\n');
    if (endIndex != std::string::npos){
		ret = input.substr(startIndex, endIndex - 5);
	}
	return ret;
}

void printStringNoP(const char* str, std::size_t length) {
    for (std::size_t i = 0; i < length; ++i) {
        if (std::isprint(static_cast<unsigned char>(str[i]))) {
            std::cout << str[i]  << std::flush;
        } else {
            std::cout << "\\x" << std::hex << static_cast<int>(str[i])  << std::flush;
            std::cout << std::dec;
        }
    }
	std::cout << std::endl << std::flush;
}

std::string removeCRLF(const char* buffer){
    std::string input = buffer;
	std::string ret = input.substr(0, input.length() - 1);
    return ret;
}