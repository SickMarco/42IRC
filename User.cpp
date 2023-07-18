/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   User.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/13 18:09:11 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/18 18:55:31 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "User.hpp"

User::User() : clientSocket(-1) {}

User::~User(){}

int User::getSocket() const { return this->clientSocket; }

void User::setSocket(const int& newSocket) { this->clientSocket = newSocket; }

void User::setIP(char* IP){ this->ServerIP = IP; }

void User::setNick(char* input) { nick = trimMessage(input, 5); }

void User::setUser(char* newUser) { this->user = newUser; }

std::string User::getNick() const { return nick; }

std::string User::getUser() const { return user; }

/* void User::changeNickname(std::string newNick){
	std::string message;
	newNick.erase(std::remove(newNick.begin(), newNick.end(), '\n'), newNick.end());
	std::cout << "NEWNICK: " << newNick << std::endl;
	message = ":" + nick + "!" + "c2r3p5.42firenze.it@" + ServerIP + " NICK :" + newNick + "\r\n";
	std::cout << message << std::endl;
	send(clientSocket, message.c_str(), strlen(message.c_str()), 0);
	nick = newNick;
} */