/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/15 19:37:31 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/27 16:02:40 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "User.hpp"

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
    std::string ret = buffer;

    if (!ret.empty() && ret[ret.length() - 1] == '\n')
        ret.resize(ret.length() - 1);
    if (ret.length() > 1 && ret[ret.length() - 1] == '\r')
        ret.resize(ret.length() - 1);
    return ret;
}

int findClient(std::vector <User> chClients, User user)
{
    std::vector <User> ::iterator it = chClients.begin();
    for (int i = 0; it != chClients.end(); it++)
    {;
        if (*it == user)
            return i;
        i++;
    }
    return -1;
}

int findClientByName(std::vector <User> chClients, std::string name)
{
    std::vector <User> ::iterator it = chClients.begin();
    for (int i = 0; it != chClients.end(); it++)
    {
        if (it->getNick() == name)
            return i;
        i++;
    }
    return -1;
}

void printUsers(std::vector<User> vec)
{
    std::vector<User> ::iterator it = vec.begin();
    for (; it != vec.end(); it++)
    {
        std::cout << "\t'" << it->getNick() << "'" << std::endl;
    }
}