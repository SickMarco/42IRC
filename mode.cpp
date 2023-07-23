/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mode.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/23 20:40:30 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/23 23:31:07 by mbozzi           ###   ########.fr       */
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

std::string extractUsername(const std::string& buffer) {
    size_t startPos = buffer.find_first_of("+-");
    if (startPos != std::string::npos) {
        size_t endPos = buffer.find('\n', startPos);
        if (endPos != std::string::npos) {
            std::string username = buffer.substr(startPos + 3, endPos - endPos - 3);
            return username;
        }
    }
    return "";
}

void Channels::setModeOperator(const User& user, std::string buffer, const std::string& flag){
	std::string newOper = extractUsername(buffer);
	std::string channelName = std::strtok(&buffer[6], " ");
	if (checkOperator(user, channelName) == false)
		return ;
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	if (it != channels.end()){
		if (!flag.compare("+o"))
			it->second.operators.push_back(user);
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
		std::string message = serverName + " 324 " + user.getNick() + " #" + channelName + " " + flag + "\r\n";
		sendToAll(channelName, message);
	}
}