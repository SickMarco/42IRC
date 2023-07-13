/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/11 17:27:51 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/13 20:46:31 by mbozzi           ###   ########.fr       */
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

#include "Irc.hpp"
#include "User.hpp"


#define SERVER_PORT 6667

class Server
{
private:
	//SERVER INIT
	const int port;
	const std::string serverPassword;
	std::string userPassword;

	//SOCKET
	int serverSocket;
	struct sockaddr_in serverAddr;

	void socketInit();
	void binding();

public:
	Server(const int& port, const std::string& password);
	~Server();

	void start();
	void tester();
};

#endif