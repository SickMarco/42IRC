/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/18 17:31:02 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/19 15:07:30 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::joinChannel(std::string channelName, User &client)
{
    std::map<std::string, Channel >::iterator it = channels.find(channelName);
	if (it != channels.end())
        it->second.clients.push_back(client);                                           // Channel exists, add the client to the channel participants
	else {
        Channel newChannel;                                                             // Channel doesn't exist, create a new channel and add the client
        newChannel.clients.push_back(client);
        channels[channelName] = newChannel;
    }
    client.channelsJoined.push_back(channelName);
    // JOIN MESSAGE SEQUENCE
    std::string join = ":" + client.getNick() + "!" + client.getNick() + "@" +  std::string(IP) + " JOIN #" + channelName + "\r\n";
    std::string RPL_TOPIC = ":" + std::string(IP) + " 332 " + client.getNick() + " #" + channelName + " :" + "\r\n";
    std::string RPL_NAMREPLY = ":" + std::string(IP) + " 353 " + client.getNick() + " = #" +channelName + " :";
    std::string RPL_ENDOFNAMES = ":" + std::string(IP) + " 366 " + client.getNick() + " #" + channelName + " :End of NAMES list\r\n";

    it = channels.find(channelName);
	for (size_t i = 0; i < it->second.clients.size(); ++i)
        if (it->second.clients[i].getSocket() != -1)
            send(it->second.clients[i].getSocket(), join.c_str(), join.length(), 0);    //SEND NEW USER JOIN TO ALL USERS

    std::vector<User>::iterator itUSer = it->second.clients.begin();                    //BUILD RPL_NAMREPLY MESSAGE
    for (;itUSer != it->second.clients.end(); ++itUSer)
    {
        RPL_NAMREPLY += itUSer->getNick();
        ((itUSer + 1) != it->second.clients.end()) ? RPL_NAMREPLY += " " : RPL_NAMREPLY += "\r\n";
    }

    if (!it->second.topic.empty())
		send(client.getSocket(), RPL_TOPIC.c_str(), RPL_TOPIC.length(), 0);
    send(client.getSocket(), RPL_NAMREPLY.c_str(), RPL_NAMREPLY.length(), 0);
    send(client.getSocket(), RPL_ENDOFNAMES.c_str(), RPL_ENDOFNAMES.length(), 0);
}

void Server::leaveChannel(std::string channelName, User& client, std::string message)
{
    // Check if the channel exists
    std::map<std::string, Channel>::iterator it = channels.find(channelName);
    (void )message;
    if (it != channels.end())
    {
        std::string PART = ":" + client.getNick() + "!" + client.getUser() + "@" + hostname +" PART :#" + channelName + "\r\n";
        printStringNoP(PART.c_str(), PART.length());
        //notify all channel participants
        std::vector<User>::iterator itc = it->second.clients.begin();
        for (; itc != it->second.clients.end(); ++itc)
            send(itc->getSocket(), PART.c_str(), PART.length(), 0);
        // Channel exists, remove the client from the channel participants
        std::vector<User> & channelClients = it->second.clients;
        channelClients.erase(std::remove(channelClients.begin(), channelClients.end(), client), channelClients.end());
        // Update client channel list
        client.channelsJoined.erase(std::remove(client.channelsJoined.begin(), client.channelsJoined.end(), channelName), client.channelsJoined.end());
    }
    
}