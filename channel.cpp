/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/18 17:31:02 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/21 17:05:35 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::joinChannel(std::string channelName, User &user)
{
    bool setOp = false;
    if (std::strchr(channelName.c_str(), ' '))
        channelName = std::strtok(&channelName[0], " ");
    //else if (std::strchr(channelName.c_str(), ',')) MULTICHANNEL
    std::map<std::string, Channel >::iterator it = channels.find(channelName);
	if (it != channels.end()) {                                                         // Channel exists, add the user to the channel participants 
        std::vector<User>::iterator userIt = it->second.clients.begin();
        for (; userIt != it->second.clients.end(); ++userIt) {
            if (userIt->getNick() == user.getNick()) {
                *userIt = user;
                break;
            }
    	}
        if (userIt == it->second.clients.end())
            it->second.clients.push_back(user);
    }                                     
	else {
        Channel newChannel;                                                             // Channel doesn't exist, create a new channel and add the user
        newChannel.clients.push_back(user);
        newChannel.operators.push_back(user.getNick());
        channels[channelName] = newChannel;
        setOp = true;
    }
    user.getChannelsJoined().push_back(channelName);
    // JOIN MESSAGE SEQUENCE
    std::string join = ":" + user.getNick() + "!" + user.getUser() + "@" + hostname + " JOIN #" + channelName + "\r\n";
    std::string RPL_TOPIC = serverName + " 332 " + user.getNick() + " #" + channelName + " :" + "\r\n";
    std::string RPL_NAMREPLY = serverName + " 353 " + user.getNick() + " = #" +channelName + " :";
    std::string RPL_ENDOFNAMES = serverName + " 366 " + user.getNick() + " #" + channelName + " :End of NAMES list\r\n";
    //SEND NEW USER JOIN TO ALL USERS
    it = channels.find(channelName);
	for (size_t i = 0; i < it->second.clients.size(); ++i)
        if (it->second.clients[i].getSocket() != -1)
            send(it->second.clients[i].getSocket(), join.c_str(), join.length(), 0);    
    //BUILD RPL_NAMREPLY MESSAGE
    std::vector<User>::iterator itUSer = it->second.clients.begin();                    
    for (;itUSer != it->second.clients.end(); ++itUSer)
    {
        RPL_NAMREPLY += itUSer->getNick();
        ((itUSer + 1) != it->second.clients.end()) ? RPL_NAMREPLY += " " : RPL_NAMREPLY += "\r\n";
    }
    if (!it->second.topic.empty())
		send(user.getSocket(), RPL_TOPIC.c_str(), RPL_TOPIC.length(), 0);
    send(user.getSocket(), RPL_NAMREPLY.c_str(), RPL_NAMREPLY.length(), 0);
    send(user.getSocket(), RPL_ENDOFNAMES.c_str(), RPL_ENDOFNAMES.length(), 0);
    if (setOp == true) {
        std::string setOperator = serverName + " MODE #" + channelName + " +o " + user.getNick() + "\r\n";
        send(user.getSocket(), setOperator.c_str(), setOperator.length(), 0);
    }
    else {
        it = channels.find(channelName);
        std::vector<std::string>::iterator opIt = it->second.operators.begin();
        for (; opIt != it->second.operators.end(); ++opIt){
            std::string sendOperator = serverName + " MODE #" + channelName + " +o " + *opIt + "\r\n";
            printStringNoP(sendOperator.c_str(), sendOperator.length());
            send(user.getSocket(), sendOperator.c_str(), sendOperator.length(), 0);
        }
    }
}

void Server::leaveChannel(std::string channelName, User& user, std::string message)
{
    // Check if the channel exists
    std::map<std::string, Channel>::iterator it = channels.find(channelName);
    (void )message;
    if (it != channels.end())
    {
        std::string PART = ":" + user.getNick() + "!" + user.getUser() + "@" + hostname +" PART :#" + channelName + "\r\n";
        //notify all channel participants
        std::vector<User>::iterator itc = it->second.clients.begin();
        for (; itc != it->second.clients.end(); ++itc)
            send(itc->getSocket(), PART.c_str(), PART.length(), 0);
        // Channel exists, remove the user from the channel participants
        std::vector<User> & channelusers = it->second.clients;
        channelusers.erase(std::remove(channelusers.begin(), channelusers.end(), user), channelusers.end());
        // Update user channel list
        user.getChannelsJoined().erase(std::remove(user.getChannelsJoined().begin(), user.getChannelsJoined().end(), channelName), user.getChannelsJoined().end());
    }
}