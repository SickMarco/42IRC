/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mode.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/23 20:40:30 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/31 15:15:30 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Channels.hpp"

std::string Server::findMode(std::string buffer){
	size_t startPos = buffer.find_first_of("+-");
	std::string mode;
    if (startPos != std::string::npos) {
		std::stringstream ss(&(buffer[startPos]));
		ss >> mode;
        return mode;
    }
    return "";
}

void Server::modeHandler(const User& user, std::string buffer){
	std::string mode = findMode(buffer);
	if (mode.empty())
		return ;
	else if (mode.find('o') != std::string::npos)
		channels.setModeOperator(user, buffer, mode);
	else if (mode.find('t') != std::string::npos)
		channels.setModeTopic(user, std::strtok(&buffer[6], " "), mode);
	else if (mode.find('i') != std::string::npos)
		channels.setModeInviteOnly(user, std::strtok(&buffer[6], " "), mode);
	else if (mode.find('k') != std::string::npos)
		channels.setModeKey(user, &(buffer[6]), mode);
	else if (mode.find('l') != std::string::npos)
		channels.setModeUserLimit(user, buffer, mode);
}

std::string extractNick(const std::string& buffer) {
    size_t startPos = buffer.find_first_of("+-");
	std::string username;
    if (startPos != std::string::npos) {
		std::stringstream ss(&(buffer[startPos]));
		ss >> username;
		ss >> username;
        return removeCRLF(username.c_str());
    }
    return "";
}

void Channels::setModeOperator(const User& user, std::string buffer, const std::string& flag){
	std::string newOper = extractNick(buffer);
	std::string channelName = std::strtok(&buffer[6], " ");
	if (checkOperator(user, channelName) == false)
		return ;
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	if (it != channels.end()){
		int ind = findClientByName(it->second.clients, newOper);
		if (ind == -1)
			return ;
		if (!flag.compare("+o"))
			it->second.operators.push_back(it->second.clients[ind]);
		else if (!flag.compare("-o"))
			it->second.operators.erase(std::find(it->second.operators.begin(), it->second.operators.end(), user));
		std::string setOperator = serverName + " MODE #" + channelName + " " + flag + " " +  newOper + "\r\n";
		sendToAll(channelName, setOperator);
	}
}

void Channels::setModeTopic(const User& user, const std::string& channelName, const std::string& flag){
	if (checkOperator(user, channelName) == false)
		return ;
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	if (it != channels.end()){
		if (!flag.compare("+t"))
			it->second.topicMode = true;
		else if (!flag.compare("-t"))
			it->second.topicMode = false;
		else
			return ;
		std::string mode = serverName + " 324 " + user.getNick() + " #" + channelName + " " + flag + "\r\n";
		sendToAll(channelName, mode);
	}
}

void Channels::setModeInviteOnly(const User& user, const std::string& channelName, const std::string& flag){
	if (checkOperator(user, channelName) == false)
		return ;
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	if (it != channels.end()){
		if (!flag.compare("+i"))
			it->second.inviteOnly = true;
		else if (!flag.compare("-i"))
			it->second.inviteOnly = false;
		else
			return ;
		std::string mode = serverName + " 324 " + user.getNick() + " #" + channelName + " " + flag + "\r\n";
		sendToAll(channelName, mode);
	}
}

void Channels::setModeUserLimit(const User& user, std::string buffer, const std::string& flag){
	std::string channelName = std::strtok(&buffer[6], " ");
	std::string maxStr , mode;
	int max;
	if (checkOperator(user, channelName) == false)
		return ;
	size_t n = buffer.find_first_of("0123456789");
	if (n == buffer.npos && !flag.compare("+l")) {
		std::string ERR_NEEDMOREPARAMS = serverName + " 461 " + user.getNick() + " #" + channelName + " " + flag + " :Not enough parameters\r\n";
		send(user.getSocket(), ERR_NEEDMOREPARAMS.c_str(), ERR_NEEDMOREPARAMS.length(), 0);
		return;
	}
	else if (!flag.compare("+l")){
		max = atoi(std::strtok(&buffer[n], "\n"));
		std::stringstream ss;
    	ss << max;
    	maxStr = ss.str();
		mode = serverName + " 324 " + user.getNick() + " #" + channelName + " " + flag + " " + maxStr + "\r\n";
	}
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	if (it != channels.end()){
		if (!flag.compare("+l")) {
			it->second.userLimit = true;
			it->second.userMax = max;
		}
		else if (!flag.compare("-l")) {
			it->second.userLimit = false;
			it->second.userMax = 0;
			mode = serverName + " 324 " + user.getNick() + " #" + channelName + " " + flag + "\r\n"; 
		}
		else
			return ;
		sendToAll(channelName, mode);
	}
}

void Channels::setModeKey(const User& user, std::string buffer, std::string mode)
{
	std::string channelName, chPass, err, garbage;
	std::string ERR_NOSUCHCHANNEL = "ERR_NOSUCHCHANNEL :" + user.getNick() + " #" + channelName + "\r\n";

	std::istringstream iss(buffer);
	iss >> channelName >> garbage >> chPass;

	if (chPass.empty() && !mode.compare("+k")){
		std::string ERR_NEEDMOREPARAMS = serverName + " 461 " + user.getNick() + " #" + channelName + " " + mode + " :Not enough parameters\r\n";
		send(user.getSocket(), ERR_NEEDMOREPARAMS.c_str(), ERR_NEEDMOREPARAMS.length(), 0);
		return ;
	}
	if (channelExist(channelName) == false) 
    {
        err = ERR_NOSUCHCHANNEL;
        send(user.getSocket(),  err.c_str(), err.length(), 0);
        return ;
    }
	if (checkOperator(user, channelName) == false)
		return ;
	
	if (mode == "+k")
		channels[channelName].passKey = chPass;
	else if (mode == "-k")
		channels[channelName].passKey.clear();

	std::string msg = serverName + " 324 " + user.getNick() + " #" + channelName + " " + mode + " :" + chPass + "\r\n";
	sendToAll(channelName, msg);
}