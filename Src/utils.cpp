/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/15 19:37:31 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/31 19:58:19 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "User.hpp"

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

std::vector<std::string> split(std::string s, char delimiter)
{
    std::vector<std::string> splitted;
    std::string token;
    std::istringstream tokenStream(s);

    while (std::getline(tokenStream, token, delimiter))
        splitted.push_back(token);
    return splitted;
}

std::string extractNick(const std::string& buffer) {
    size_t startPos = buffer.find_first_of("+-");
	std::string username;
    if (startPos != std::string::npos) {
		std::stringstream ss(&(buffer[startPos]));
		ss >> username;
		ss >> username;
        return removeCRLF(username.c_str());
    }
    return "";
}