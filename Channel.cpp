/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/18 17:31:02 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/18 17:32:02 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::joinChannel(std::string channelName, User client)
{
    std::map<std::string, Channel >::iterator it = channels.find(channelName);
	if (it != channels.end())
        it->second.clients.push_back(client); // Channel exists, add the client to the channel participants
	else 
	{
        Channel newChannel; // Channel doesn't exist, create a new channel and add the client
        newChannel.clients.push_back(client);
        channels[channelName] = newChannel;
    }
    // JOIN MESSAGE SEQUENCE
    std::string join = ":" + client.getNick() + "!" + client.getNick() + "@" +  std::string(IP) + " JOIN #" + channelName + "\r\n";
    std::string RPL_TOPIC = ":" + std::string(IP) + " 332 " + client.getNick() + " #" + channelName + " :" + "\r\n";
    std::string RPL_NAMREPLY = ":" + std::string(IP) + " 353 " + client.getNick() + " = #" +channelName + " :";
    std::string RPL_ENDOFNAMES = ":" + std::string(IP) + " 366 " + client.getNick() + " #" + channelName + " :End of NAMES list\r\n";

    it = channels.find(channelName);
	for (size_t i = 0; i < it->second.clients.size(); ++i)
        if (it->second.clients[i].getSocket() != -1)
            send(it->second.clients[i].getSocket(), join.c_str(), join.length(), 0);  //SEND NEW USER JOIN TO ALL USERS

    std::vector<User>::iterator itUSer = it->second.clients.begin(); //BUILD RPL_NAMREPLY MESSAGE
    for (;itUSer != it->second.clients.end(); ++itUSer)
    {
        RPL_NAMREPLY += itUSer->getNick();
        ((itUSer + 1) != it->second.clients.end()) ? RPL_NAMREPLY += " " : RPL_NAMREPLY += "\r\n";
    }

    send(client.getSocket(), RPL_TOPIC.c_str(), RPL_TOPIC.length(), 0);
    send(client.getSocket(), RPL_NAMREPLY.c_str(), RPL_NAMREPLY.length(), 0);
    send(client.getSocket(), RPL_ENDOFNAMES.c_str(), RPL_ENDOFNAMES.length(), 0);
}

int Server::messageToChannel(User& user, std::string buffer)
{
    std::string channelName = buffer.substr(1, buffer.find(' '));
    channelName = channelName.substr(0, channelName.length() - 1);
    std::string mex = buffer.substr(channelName.length() + 1, std::string::npos);
      std::cout << "2 :" << "\'" + channelName + "\'" << std::endl;

    // Find the channel
    std::map<std::string, Channel >::iterator it = channels.find(channelName);
    std::cout << "SIZE " << channels.size() << std::endl;
    printStringNoP(channelName.c_str(), channelName.length());
	for (std::map<std::string, Channel >::iterator start = channels.begin(); start != channels.end(); ++start)
    {
		printStringNoP(start->first.c_str(), start->first.length());
    }
    
    if (it != channels.end())
    {
        // Channel exists, send the message to all clients in the channel
        std::vector<User> channelClients = it->second.clients;
        std::string privmsg = ":" + user.getNick() + " PRIVMSG #" + channelName + " " + mex.substr(0, mex.length()) + "\r\n";

        for (size_t i = 0; i < channelClients.size(); ++i)
        {
            if (channelClients[i].getSocket() != -1 && channelClients[i].getSocket() != user.getSocket())
            {
                std::cout << "CLIENT FOUND" << std::endl;
                send(channelClients[i].getSocket(), privmsg.c_str(), privmsg.length(), 0);
            }
        }
        printStringNoP(privmsg.c_str(), privmsg.length());
        return 1;
    }
    else
        std::cout << "CHANNEL NOT FOUND" << std::endl;
    return 0;
}