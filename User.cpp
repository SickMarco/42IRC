/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   User.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/13 18:09:11 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/20 19:05:57 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "User.hpp"

User::User() : clientSocket(-1) {}

User::~User(){}

int User::getSocket() const { return this->clientSocket; }

void User::setSocket(const int& newSocket) { this->clientSocket = newSocket; }

void User::setIP(const std::string& IP){ this->ServerIP = IP; }

void User::setNick(std::string input) { nick = input; }

void User::setUser(std::string newUser) { this->user = newUser; }

std::string User::getNick() const { return nick; }

std::string User::getUser() const { return user; }

std::vector<std::string>& User::getChannelsJoined() { return this->channelsJoined; }