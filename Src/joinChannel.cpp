/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   joinChannel.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/31 15:51:36 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/31 15:51:37 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channels.hpp"

void Channels::multiChannelJoin(User& user, std::string buffer)
{
    std::stringstream ss(buffer);
    std::string channelNames;
    ss >> channelNames;
    std::string keys;
    ss >> keys;
    std::vector <std::string> chNames = split(channelNames, ',');
    std::vector <std::string> chKeys = split(keys, ',');
    std::string key;
    
    std::vector <std::string> ::iterator it = chNames.begin();
    std::vector <std::string> ::iterator it2 = chKeys.begin();
    for (; it != chNames.end(); it++)
    {
        if (it2 == chKeys.end())
            key.clear();
        else
            key = *it2;
        std::string name = *it;
        if (name.length() >= 3)
            joinChannel(user, &(name[1]), key);
        if (it2 != chKeys.end())
            it2++;
    }
}

int Channels::joinChannel(User& user, std::string channelName, std::string key)
{
    if (checkChannelModes(user, channelName, key))
        return 1;
    bool setOp = false;
    if (channelName.find(' ') != channelName.npos)
        channelName = std::strtok(&channelName[0], " ");
    if (channelExist(channelName) == true)
        addUserToChannel(user, channelName);
    else
        createNewChannel(user, channelName, setOp);
    user.getChannels().push_back(channelName);
    joinMessageSequence(user, channelName);
    channelOperators(user, channelName, setOp);
    return 0;
}

void Channels::addUserToChannel(User& user, std::string channelName)
{
    std::map<std::string, Channel >::iterator it = channels.find(channelName);
    if (it != channels.end()) {
        // Channel exists, add the user to the channel participants
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
}

void Channels::joinMessageSequence(const User& user, const std::string& channelName) {
    std::string join = ":" + user.getNick() + "!" + user.getUser() + "@" + hostname + " JOIN #" + channelName + "\r\n";
    std::string RPL_NAMREPLY = serverName + " 353 " + user.getNick() + " = #" + channelName + " :";
    std::string RPL_ENDOFNAMES = serverName + " 366 " + user.getNick() + " #" + channelName + " :End of NAMES list\r\n";
    // Send JOIN message to all users in the channel
    sendToAll(channelName, join);
    // Build RPL_NAMREPLY message
    std::map<std::string, Channel >::iterator it = channels.find(channelName);
    std::vector<User>::iterator itUser = it->second.clients.begin();
    for (; itUser != it->second.clients.end(); ++itUser) {
        RPL_NAMREPLY += itUser->getNick();
        ((itUser + 1) != it->second.clients.end()) ? RPL_NAMREPLY += " " : RPL_NAMREPLY += "\r\n";
    }
    // Check if topic is setted
    if (!it->second.topic.empty()){
        std::string RPL_TOPIC = serverName + " 332 " + user.getNick() + " #" + channelName + " :" + it->second.topic + "\r\n";
        send(user.getSocket(), RPL_TOPIC.c_str(), RPL_TOPIC.length(), sndFlags);
    }
    //Check if channels mode enabled
    if (it->second.topicMode == true || it->second.inviteOnly == true || it->second.userLimit == true || !it->second.passKey.empty()){
        std::string RPL_CHANNELMODEIS = serverName + " 324 " + user.getNick() + " #" + channelName + " +";
        std::string param = " ";
        if (it->second.topicMode == true)
            RPL_CHANNELMODEIS += 't';
        if (it->second.inviteOnly == true)
            RPL_CHANNELMODEIS += 'i';
        if (it->second.userLimit == true){
            std::stringstream ss;
            ss << it->second.userMax;
            std::string num = ss.str();
            param += num;
            RPL_CHANNELMODEIS += 'l';
        }
        if (!it->second.passKey.empty()){
            if (RPL_CHANNELMODEIS.find("i") != RPL_CHANNELMODEIS.npos)
                param += " ";
            RPL_CHANNELMODEIS += "k";
            param += it->second.passKey;
        }
        RPL_CHANNELMODEIS += param + "\r\n";
        send(user.getSocket(), RPL_CHANNELMODEIS.c_str(), RPL_CHANNELMODEIS.length(), sndFlags);
    }
    send(user.getSocket(), RPL_NAMREPLY.c_str(), RPL_NAMREPLY.length(), sndFlags);
    send(user.getSocket(), RPL_ENDOFNAMES.c_str(), RPL_ENDOFNAMES.length(), sndFlags);
}
