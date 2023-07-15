/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   User.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/13 18:09:11 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/15 19:57:29 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "User.hpp"

User::User() : clientSocket(-1) {}

User::~User(){}

/* void User::socketAccept(const int serverSocket){
	clientAddrLen = sizeof(clientAddr);
	clientSocket = accept(serverSocket, (struct sockaddr*) &clientAddr, &clientAddrLen);
	if (clientSocket < 0)
		if (errno != EWOULDBLOCK && errno != EAGAIN)
			throw std::runtime_error("Client accept error");
} */

int User::getSocket() const { return this->clientSocket; }

void User::setSocket(const int& newSocket) { this->clientSocket = newSocket; }

void User::setIP(char* IP){ this->ServerIP = IP; }

void User::setNick(char* input) {
	nick = trimMessage(input, 5);
	std::cout << "NICKNAME : " << nick << std::endl;
}

/* void User::changeNickname(std::string newNick){
	std::string message;
	newNick.erase(std::remove(newNick.begin(), newNick.end(), '\n'), newNick.end());
	std::cout << "NEWNICK: " << newNick << std::endl;
	message = ":" + nick + "!" + "c2r3p5.42firenze.it@" + ServerIP + " NICK :" + newNick + "\r\n";
	std::cout << message << std::endl;
	send(clientSocket, message.c_str(), strlen(message.c_str()), 0);
	nick = newNick;
} */

void User::joinChannel(const char* buffer){
	std::string join = ":" + nick + " JOIN :" + trimMessage(buffer, 5) + "\r\n";
	send(clientSocket, join.c_str(), join.length(), 0);
}