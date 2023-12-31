/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   User.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/13 18:09:11 by mbozzi            #+#    #+#             */
/*   Updated: 2023/08/01 14:59:07 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "User.hpp"

User::User() : clientAddrLen(sizeof(clientAddr)), clientSocket(-1), serverPass(""), msgBuffer(""), login(false) {
	memset(buffer, 0, sizeof(buffer));
}

User::~User(){}

sockaddr_in& User::getAddr() { return this->clientAddr;}

socklen_t& User::getAddrLen() { return this->clientAddrLen; }

int User::getSocket() const { return this->clientSocket; }

void User::setSocket(const int& newSocket) { this->clientSocket = newSocket; }

void User::setIP(const std::string& IP){ this->ServerIP = IP; }

void User::setNick(std::string input) { this->nick = input; }

void User::setUser(std::string newUser) { this->user = newUser; }

std::string User::getNick() const { return this->nick; }

std::string User::getUser() const { return this->user; }

std::vector<std::string>& User::getChannels() { return this->channelsJoined; }

bool User::operator==(const User& other) const {return (this->clientSocket == other.clientSocket);}

User&  User::operator=(const User& src){
	if (this != &src){
		this->clientAddr = src.clientAddr;
		this->clientAddrLen = src.clientAddrLen;
		this->clientSocket = src.clientSocket;
		this->nick = src.nick;
		this->ServerIP = src.ServerIP;
	}
	return *this;
}

std::string User::getPass() const { return this->serverPass; }

void  User::setPass(const std::string& passw) { this->serverPass = passw; }

void User::reset(){
	this->nick = "";
    this->user = "";
    this->ServerIP = "";
    this->serverPass = "";
	this->login = false;
    this->msgBuffer.clear();
	this->channelsJoined.clear();
    memset(&(this->getAddr()), 0, sizeof(this->getAddr()));
    memset(&(this->getAddrLen()), 0, sizeof(this->getAddrLen()));
    memset(this->buffer, 0, sizeof(this->buffer));
}