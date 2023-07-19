/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/11 17:27:51 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/19 15:04:20 by mbozzi           ###   ########.fr       */
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
#include <sstream>
#include <map>
#include <vector>
#include <algorithm>

#include "User.hpp"
#include "Irc.hpp"


#define SERVER_PORT 6667
#define MAX_CLIENTS 100

struct Channel {
	std::string topic;
	std::vector<User> clients;
};

class Server
{
private:
	//SERVER INIT
	const std::string serverName;
	const std::string serverPassword;
	std::string userPassword;
	const int port;
	char hostname[256];
	char *IP;

	std::vector<User> clients;
	std::map<std::string, Channel > channels;	//mappa il nome del canale ad un vettore di clients che ne fanno parte

	//SOCKET
	int serverSocket;
	struct sockaddr_in serverAddr;

	void getMyIP();
	void socketInit();
	void binding();
	int messageToPrivate(User& user, std::string buffer);
	int messageToChannel(User& user, std::string buffer);

	void joinChannel(std::string channelName, User &client);
	void leaveChannel(std::string channelName, User &client, std::string message);

	void newClientHandler(struct pollfd* fds, int& numClients);
	void newClientConnected(User& user);
	void messageHandler(User& user);

public:
	Server(const int& port, const std::string& password);
	~Server();

	void run();
	friend std::string trimMessage(const char* buffer, size_t startIndex);
	friend void printStringNoP(const char* str, std::size_t length);
};

#endif