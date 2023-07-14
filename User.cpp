/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   User.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/13 18:09:11 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/14 18:21:10 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "User.hpp"

User::User() : clientSocket(-1), nick("") {}

User::~User(){}

void User::socketAccept(const int serverSocket){
	clientAddrLen = sizeof(clientAddr);
	clientSocket = accept(serverSocket, (struct sockaddr*) &clientAddr, &clientAddrLen);
	if (clientSocket < 0)
		if (errno != EWOULDBLOCK && errno != EAGAIN)
			throw std::runtime_error("Client accept error");
}

int User::getSocket() const { return this->clientSocket; }

void User::setIP(char* IP){ this->ServerIP = IP; }

void User::setNick(char* input) {
	const char delimiter[] = " \r\n";

    char* token = std::strtok(input, delimiter);
    while (token != NULL) {
        if (std::strcmp(token, "NICK") == 0) {
            token = std::strtok(NULL, delimiter);
            if (token != NULL)
				nick = std::string(token);
            break;
        }
        token = std::strtok(NULL, delimiter);
    }
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