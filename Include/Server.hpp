/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/11 17:27:51 by mbozzi            #+#    #+#             */
/*   Updated: 2023/08/01 12:14:39 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <stdio.h>
#include <poll.h>
#include <sys/epoll.h>
#include <sstream>
#include <map>
#include <vector>
#include <algorithm>

#include "User.hpp"
#include "Channels.hpp"
#include "Socket.hpp"


#define SERVER_PORT 6667
#define MAX_CLIENTS 100

class Server
{
private:
	//SERVER INIT
	const std::string serverName;
	const std::string serverPassword;
	const int port;
	std::string userPassword;
	std::string hostname;
	std::string IP;
	bool isServerRunning;
	//SOCKET
	Socket skt;
	int epollFd;
	int addSocketToEpoll(int newSocket);
	//CLIENTS
	std::vector<User> clients;
	Channels channels;
	int clientsConnected;
	int newClientHandler();
	int findClientIndex(int clientSocket);
	void messageHandler(User& user);
	void loginHandler(User& user, std::vector<std::string>& cmds);
	bool checkPassword(User& user, const std::string& PASS);
	void welcomeMsg(const User& user);
	void commandHandler(User &user);
	//CMDS
	int messageToPrivate(User& user, std::string buffer);
	void quit(char * buffer, User &user);
	int changeNick(std::string buffer, User &user, int flag);
	std::string findMode(std::string buffer);
	void invite(std::string buffer, User &user);
	void modeHandler(const User& user, std::string buffer);
	void kick(std::string buffer, User &user);

public:
	Server(const int& port, const std::string& password);
	~Server();

	void run();
};

std::string removeCRLF(const char* buffer);
int 		findClient(std::vector <User> chClients, User user);
int 		findClientByName(std::vector <User> chClients, std::string name);
void		printUsers(std::vector<User> vec);

#endif