/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   User.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/13 18:09:03 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/13 20:24:56 by mbozzi           ###   ########.fr       */
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

class User
{
private:
	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen;
	int clientSocket;
	std::string nick;
	std::string realname;
	std::string host;
	//char buffer[1024];

public:
	User();
	~User();

	void socketAccept(const int serverSocket);
	int getSocket() const;
	void setNick();
	void changeNickname(const std::string& newNick);
};

#endif