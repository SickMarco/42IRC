/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/11 17:27:51 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/15 20:06:12 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

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
#include <netdb.h>

#include "User.hpp"
#include "Irc.hpp"


#define SERVER_PORT 6667
#define MAX_CLIENTS 100

class Server
{
private:
	//SERVER INIT
	const int port;
	const std::string serverPassword;
	std::string userPassword;
	char hostname[256];
	char *IP;

	//SOCKET
	int serverSocket;
	struct sockaddr_in serverAddr;

	void getMyIP();
	void socketInit();
	void binding();

public:
	Server(const int& port, const std::string& password);
	~Server();

	void tester();
	void newClientConnected(User& user);
	void messageHandler(User& user);
	friend std::string trimMessage(const char* buffer, size_t startIndex);
	friend void printStringNoP(const char* str, std::size_t length);
};

#endif