/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/18 17:31:02 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/20 19:06:33 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::joinChannel(std::string channelName, User &client)
{
    if (std::strchr(channelName.c_str(), ' '))
        channelName = std::strtok(&channelName[0], " ");
    //else if (std::strchr(channelName.c_str(), ',')) MULTICHANNEL
    std::map<std::string, Channel >::iterator it = channels.find(channelName);
	if (it != channels.end())                                                           // Channel exists, add the client to the channel participants
    {   
        std::cout << "JOIN 1" << std::endl;
        it->second.clients.push_back(client);
        std::cout << "JOIN 1" << std::endl;
        std::string userJoined = ":" + client.getNick() + " JOIN #" + channelName + "\r\n";
    /*     std::vector<int>::iterator opIt = it->second.operators.begin();
        for(; opIt != it->second.operators.end(); ++opIt)
        {   
            printStringNoP(userJoined.c_str(), userJoined.length());
            send(*opIt, userJoined.c_str(), userJoined.length(), 0);
        } */
    }                                       
	else {
        std::cout << "JOIN 2" << std::endl;
        Channel newChannel;                                                             // Channel doesn't exist, create a new channel and add the client
        newChannel.clients.push_back(client);
        //newChannel.operators.push_back();
        channels[channelName] = newChannel;
        //std::string RPL_CHANNELMODEIS  = serverName + " 324 " + client.getNick() + " #" + channelName + " +o\r\n";
        //printStringNoP(RPL_CHANNELMODEIS.c_str(), RPL_CHANNELMODEIS.length());
        //send(client.getSocket(), RPL_CHANNELMODEIS.c_str(), RPL_CHANNELMODEIS.length(), 0);
    }
    client.getChannelsJoined().push_back(channelName);
    // JOIN MESSAGE SEQUENCE
    std::string join = ":" + client.getNick() + "!" + client.getUser() + "@" + hostname + " JOIN #" + channelName + "\r\n";
    std::string RPL_TOPIC = serverName + " 332 " + client.getNick() + " #" + channelName + " :" + "\r\n";
    std::string RPL_NAMREPLY = serverName + " 353 " + client.getNick() + " = #" +channelName + " :";
    std::string RPL_ENDOFNAMES = serverName + " 366 " + client.getNick() + " #" + channelName + " :End of NAMES list\r\n";
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
        //notify all channel participants
        std::vector<User>::iterator itc = it->second.clients.begin();
        for (; itc != it->second.clients.end(); ++itc)
            send(itc->getSocket(), PART.c_str(), PART.length(), 0);
        // Channel exists, remove the client from the channel participants
        std::vector<User> & channelClients = it->second.clients;
        channelClients.erase(std::remove(channelClients.begin(), channelClients.end(), client), channelClients.end());
        // Update client channel list
        client.getChannelsJoined().erase(std::remove(client.getChannelsJoined().begin(), client.getChannelsJoined().end(), channelName), client.getChannelsJoined().end());
    }
}