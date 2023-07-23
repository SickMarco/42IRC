/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/22 11:44:36 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/23 22:56:22 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"

Socket::Socket() : serverSocket(-1) {}

Socket::~Socket(){}

int Socket::getSocket() const { return this->serverSocket; }

//Socket init with non-blocking I/O
void Socket::socketInit(std::string IP){
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) 
		throw std::runtime_error("Socket failed");
    if (fcntl(serverSocket, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("Socket non-blocking error");
	int reuseAddr = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr)) < 0)
		throw std::runtime_error("Socket can't be reused");
	memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(IP.c_str());
    //controlla puliza serverAddr
}

void Socket::binding(){
    if (bind(serverSocket, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) < 0)
      throw std::runtime_error("Binding error");
    if (listen(serverSocket, SOMAXCONN) < 0)
       throw std::runtime_error("Listening error");
}

void Socket::getMyIP(std::string& hostnm, std::string& IP){
	struct hostent *host_entry;
	char hostname[256];

	int	host = gethostname(hostname, sizeof(hostname)) == 0;
    if (host < 0)
		throw std::runtime_error("Hostname error");
   	hostnm = hostname;
	host_entry = gethostbyname(&(hostname[0]));
	if(!host_entry)
		throw std::runtime_error("Host entry error");
    IP = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
	std::memset(hostname, 0, sizeof(hostname)); 
}