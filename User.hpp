/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   User.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/13 18:09:03 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/21 11:55:23 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef USER_HPP
#define USER_HPP

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <poll.h>
#include <algorithm>
#include <map>
#include <vector>

#include "Irc.hpp"

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
	std::vector<std::string>& getChannelsJoined();

	void setNick(std::string input);
	void setUser(std::string newUser);

	friend std::string trimMessage(const char* buffer, size_t startIndex);
	friend void printStringNoP(const char* str, std::size_t length);
};

#endif