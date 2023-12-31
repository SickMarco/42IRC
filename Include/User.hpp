/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   User.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/13 18:09:03 by mbozzi            #+#    #+#             */
/*   Updated: 2023/08/01 14:51:20 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef USER_HPP
#define USER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>

class User
{
private:
	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen;
	int clientSocket;
	std::string nick;
	std::string user;
	std::string serverPass;

	std::string ServerIP;
	std::vector <std::string> channelsJoined;

public:
	User();
	~User();
	bool operator==(const User& other) const;
	User& operator=(const User& src);

	int getSocket() const;
	sockaddr_in& getAddr();
	socklen_t& getAddrLen();
	void setSocket(const int& newSocket);
	void setIP(const std::string& IP);

	std::string getPass() const;
	std::string getUser() const;
	std::string getNick() const;
	std::vector<std::string>& getChannels();

	void setNick(std::string input);
	void setUser(std::string newUser);
	void setPass(const std::string& passw);

	void reset();

	char buffer[1024];
	std::string msgBuffer;
	bool login;
};

const int sndFlags = MSG_DONTWAIT | MSG_NOSIGNAL;

std::string removeCRLF(const char* buffer);
void 		printStringNoP(const char* str, std::size_t length);

#endif