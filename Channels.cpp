/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channels.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/22 10:58:42 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/24 15:34:45 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channels.hpp"

Channels::Channels(){}

Channels::~Channels(){}

void Channels::init(const std::string& serverName, const std::string& hostname) {
    this->serverName = serverName;
    this->hostname = hostname;
}

std::map<std::string, Channel>& Channels::getChannels(){ return this->channels; }

void Channels::sendToAll(const std::string& channelName, const std::string& message){
    std::map<std::string, Channel >::iterator it = channels.find(channelName);
    for (size_t i = 0; i < it->second.clients.size(); ++i) {
        if (it->second.clients[i].getSocket() != -1) {
            send(it->second.clients[i].getSocket(), message.c_str(), message.length(), 0);
        }
    }
}

int Channels::messageToChannel(const User& user, std::string buffer) {
    std::string channelName = buffer.substr(1, buffer.find(' '));
    channelName = channelName.substr(0, channelName.length() - 1);
    std::string mex = buffer.substr(channelName.length() + 1, std::string::npos);

    // Find the channel
    std::map<std::string, Channel >::iterator it = channels.find(channelName);
    if (it != channels.end()) {
        std::vector<User> channelClients = it->second.clients;
        std::string privmsg = ":" + user.getNick() + " PRIVMSG #" + channelName + " " + mex.substr(0, mex.length()) + "\r\n";

        for (size_t i = 0; i < channelClients.size(); ++i) {
            if (channelClients[i].getSocket() != -1 && channelClients[i].getSocket() != user.getSocket())
                send(channelClients[i].getSocket(), privmsg.c_str(), privmsg.length(), 0);
        }
        return 1;
    }
    return 0;
}

void Channels::joinChannel(User& user, std::string channelName) {
    //if channel is ivite only and the user has not been invited, abort joining
    std::map <std::string, Channel>::iterator it = channels.begin();
    if (it != channels.end())
    {
        if (it->second.inviteOnly == true &&
            it->second.invitelist.find(user.getNick()) == it->second.invitelist.end())
        {

        }
        //if user has been banned from channel, abort joining
        if (it->second.banlist.find(user.getNick()) != it->second.banlist.end())
        {

        }
    }
    bool setOp = false;
    if (std::strchr(channelName.c_str(), ','))
        multiChannelJoin(user, std::strtok(&channelName[0], " "));
    else
    {
        if (std::strchr(channelName.c_str(), ' '))
            channelName = std::strtok(&channelName[0], " ");
        if (!channelExist(user, channelName))
            createNewChannel(user, channelName, setOp);
        user.getChannels().push_back(channelName);
        joinMessageSequence(user, channelName);
        channelOperators(user, channelName, setOp);
    }
}

bool Channels::channelExist(User& user, const std::string& channelName){
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
        return true;
    }
    return false;
}

void Channels::createNewChannel(const User& user, const std::string& channelName, bool& setOp){
    // Channel doesn't exist, create a new channel and add the user
    Channel newChannel;                           
    newChannel.clients.push_back(user);
    newChannel.operators.push_back(user);
    newChannel.topicMode = false;
    newChannel.inviteOnly = false;
    channels[channelName] = newChannel;
    setOp = true;
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
        send(user.getSocket(), RPL_TOPIC.c_str(), RPL_TOPIC.length(), 0);
    }
    else {
        std::string RPL_NOTOPIC = serverName + " 331 " + user.getNick() + " #" + channelName + " :No topic setted\r\n";
        send(user.getSocket(), RPL_NOTOPIC.c_str(), RPL_NOTOPIC.length(), 0);
    }
    send(user.getSocket(), RPL_NAMREPLY.c_str(), RPL_NAMREPLY.length(), 0);
    send(user.getSocket(), RPL_ENDOFNAMES.c_str(), RPL_ENDOFNAMES.length(), 0);
}

void Channels::channelOperators(const User& user, const std::string& channelName, bool& setOp){
    if (setOp == true) {
        //Set first user as operator
        std::string setOperator = serverName + " MODE #" + channelName + " +o " + user.getNick() + "\r\n";
        printStringNoP(setOperator.c_str(), setOperator.length());
        send(user.getSocket(), setOperator.c_str(), setOperator.length(), 0);
    }
    else {
        //Notify all user of channels operators
        std::map<std::string, Channel >::iterator it = channels.find(channelName);
        std::vector<User>::iterator opIt = it->second.operators.begin();
        for (; opIt != it->second.operators.end(); ++opIt) {
            std::string sendOperator = serverName + " MODE #" + channelName + " +o " + opIt->getNick() + "\r\n";
            send(user.getSocket(), sendOperator.c_str(), sendOperator.length(), 0);
        }
    }
}

 void Channels::multiChannelJoin(User& user, std::string channelName){
    std::vector<std::string> splitChannel;
    size_t pos = 0;
    size_t next;
    while ((next = channelName.find(',', pos)) != std::string::npos) {
        std::string channel = channelName.substr(pos, next - pos);
        channel.erase(std::remove(channel.begin(), channel.end(), '#'), channel.end());
        splitChannel.push_back(channel);
        pos = next + 1;
    }
    std::string lastChannel = channelName.substr(pos);
    lastChannel.erase(std::remove(lastChannel.begin(), lastChannel.end(), '#'), lastChannel.end());
    splitChannel.push_back(lastChannel);
    for (std::vector<std::string>::iterator it = splitChannel.begin(); it != splitChannel.end(); ++it)
        joinChannel(user, *it);
}

void Channels::leaveChannel(User& user, std::string channelName, std::string message)
{
    // Check if the channel exists
    std::map<std::string, Channel >::iterator it = channels.find(channelName);
    (void )message;
    if (it != channels.end())
    {
        std::string PART = ":" + user.getNick() + "!" + user.getUser() + "@" + hostname +" PART :#" + channelName + "\r\n";
        //Notify all channel users
        std::vector<User>::iterator itc = it->second.clients.begin();
        for (; itc != it->second.clients.end(); ++itc)
            send(itc->getSocket(), PART.c_str(), PART.length(), 0);
        // Channel exists, remove the user from the channel participants
        std::vector<User> & channelusers = it->second.clients;
        channelusers.erase(std::remove(channelusers.begin(), channelusers.end(), user), channelusers.end());
        // Update user channel list
        user.getChannels().erase(std::remove(user.getChannels().begin(), user.getChannels().end(), channelName), user.getChannels().end());
    }
}

bool Channels::checkOperator(const User& user, const std::string& channelName){
    std::map<std::string, Channel>::iterator it = channels.find(channelName);
    if (it != channels.end()){
        std::vector<User>::iterator userIt = it->second.operators.begin();
        for (; userIt != it->second.operators.end(); ++userIt){
            if (userIt->getNick() == user.getNick())
                return true;
        }
        std::string ERR_CHANOPRIVSNEEDED = serverName + " 482 " + user.getNick() + " #" + channelName + " :You're not channel operator\r\n";
        send(user.getSocket(), ERR_CHANOPRIVSNEEDED.c_str(), ERR_CHANOPRIVSNEEDED.length(), 0);
    }
    return false;
}

void Channels::setTopic(const User& user, const std::string& channelName, const std::string& arg){
    std::map<std::string, Channel>::iterator it = channels.find(channelName);
    if (it != channels.end()) {
        it->second.topic = arg;
        std::string RPL_TOPICSET = serverName + " 332 " + user.getNick() + " #" + channelName + " :" + arg + "\r\n";
        sendToAll(channelName, RPL_TOPICSET);
    }
}

void Channels::topic(const User& user, std::string buffer){
    size_t index1 = buffer.find('#');
    size_t index2 = buffer.find(':');

    std::string channelName = buffer.substr(index1 + 1, index2 - index1 - 2);
    std::string arg = buffer.substr(index2 + 1);

    std::map<std::string, Channel>::iterator it = channels.find(channelName);
    if (it != channels.end()){
        if (it->second.topic.empty() && !std::strchr(buffer.c_str(), ':')){
            std::string RPL_NOTOPIC = serverName + " 331 " + user.getNick() + " #" + channelName + " :No topic setted\r\n";
            // SEND ?
        }
        else if (!std::strchr(buffer.c_str(), ':')){    //SEND TOPIC
            std::string RPL_TOPIC = serverName + " 332 " + user.getNick() + " #" + channelName + " :" + it->second.topic + "\r\n";
            send(user.getSocket(), RPL_TOPIC.c_str(), RPL_TOPIC.length(), 0);
        }
        else if (it->second.topicMode == false)    //CHECK TOPIC MODE AND SET TOPIC
            setTopic(user, channelName, arg);
        else if (it->second.topicMode == true && checkOperator(user, channelName)) //TOPIC MODE ON, CHECK OPERATOR AND SET TOPIC
            setTopic(user, channelName, arg);
    }
}