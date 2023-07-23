/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mode.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/23 20:40:30 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/23 22:59:11 by mbozzi           ###   ########.fr       */
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

void Channels::setModeTopic(const User& user, const std::string& channelName, const std::string& flag){
	if (checkOperator(user, channelName) == false)
		return ;
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	if (!flag.compare("+t"))
		it->second.topicMode = true;
	else if (!flag.compare("-t"))
		it->second.topicMode = false;
	else
		return ;
	std::string message = serverName + " 324 " + user.getNick() + " #" + channelName + " " + flag + "\r\n";
	sendToAll(channelName, message);
}