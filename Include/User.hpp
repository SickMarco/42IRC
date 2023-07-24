/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   User.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/13 18:09:03 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/24 16:53:18 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef USER_HPP
#define USER_HPP

#include <iostream>
#include <string>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <map>
#include <vector>

class User
{
private:
	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen;
	int clientSocket;
	std::string nick;
	std::string user;

	std::string host;
	std::string ServerIP;
	std::vector <std::string> channelsJoined;

public:
	User();
	~User();
	bool operator==(const User& other) const;
	User& operator=(const User& src);

	int getSocket() const;
	void setSocket(const int& newSocket);
	void setIP(const std::string& IP);

	std::string getUser() const;
	std::string getNick() const;
	std::vector<std::string>& getChannels();

	void setNick(std::string input);
	void setUser(std::string newUser);
};

std::string removeCRLF(const char* buffer);
void 		printStringNoP(const char* str, std::size_t length);

#endif