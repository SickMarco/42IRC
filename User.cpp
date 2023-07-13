/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   User.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/13 18:09:11 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/13 20:47:09 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "User.hpp"

User::User() : clientSocket(-1) {}

User::~User(){}

void User::socketAccept(const int serverSocket){
	clientAddrLen = sizeof(clientAddr);
	clientSocket = accept(serverSocket, (struct sockaddr*) &clientAddr, &clientAddrLen);
	if (clientSocket < 0)
		if (errno != EWOULDBLOCK && errno != EAGAIN)
			throw std::runtime_error("Client accept error");
}

int User::getSocket() const { return this->clientSocket; }

void User::setNick() {
	char name[1024];
	ssize_t bt = recv(clientSocket, name, sizeof(name) - 1, 0);
	if (bt < 0)
		perror("Receive error");
	name[bt] = '\0';
	std::string tmp = name;
	size_t find = tmp.find("NICK") + 5;
	std::string tmp1 = tmp.substr(find , tmp.npos);
	std::string tmp2 = tmp1.substr(0 , tmp1.find('\r'));
	this->nick = tmp2;
}

void User::changeNickname(const std::string& newNick){
	std::string message = ":" + nick + "!" + "mbozzi@127.0.0.1" + " NICK :" + newNick + "\r\n";
	send(clientSocket, message.c_str(), strlen(message.c_str()), 0);
}