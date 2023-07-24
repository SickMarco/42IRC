/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mode.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/23 20:40:30 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/24 18:54:41 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Channels.hpp"

std::string Server::findMode(std::string buffer){
	size_t index = buffer.find_first_of("+-");
    if (index != std::string::npos) {
        std::string mode = buffer.substr(index, 2);
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
}

std::string extractNick(const std::string& buffer) {
    size_t startPos = buffer.find_first_of("+-");
    if (startPos != std::string::npos) {
        size_t endPos = buffer.find('\n', startPos);
        if (endPos != std::string::npos) {
            std::string username = buffer.substr(startPos + 3, endPos - startPos - 3);
            return username;
        }
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
		int ind = Server::findClientByName(it->second.clients, newOper);
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
//MODE #me +k dio\xa
void Channels::setModeKey(const User& user, std::string buffer, std::string mode)
{
	std::string channelName = buffer.substr(0, buffer.find(' '));
	std::string chPass = buffer.substr(buffer.find(mode) + 3, buffer.npos);
	chPass = chPass.substr(0, chPass.length() - 1);
	std::string ERR_NOSUCHCHANNEL = "ERR_NOSUCHCHANNEL :" + user.getNick() + " #" + channelName + "\r\n";
	std::string err;

	if (channelExist2(channelName) == false) 
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
}