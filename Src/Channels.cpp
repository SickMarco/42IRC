/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channels.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/22 10:58:42 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/25 18:37:35 by mbozzi           ###   ########.fr       */
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

int  Channels::checkChannelModes(const User& user, const std::string channelName, const std::string& key){
    //if channel is ivite only and the user has not been invited, abort joining
    std::map <std::string, Channel>::iterator it = channels.find(channelName);
    if (it != channels.end())
    {
        if (it->second.inviteOnly == true &&
           it->second.invitelist.find(user.getNick()) == it->second.invitelist.end())
        {
            std::string ERR_INVITEONLYCHAN = "ERR_INVITEONLYCHAN " + user.getNick() +  " #" + channelName + " :Cannot join channel (+i)\r\n";
            send(user.getSocket(), ERR_INVITEONLYCHAN.c_str(), ERR_INVITEONLYCHAN.length(), 0);
            return 1;
        }
    //    if (it->second.banlist.find(user.getNick()) != it->second.banlist.end()){} //if user has been banned from channel, abort joining
        if (it->second.userLimit == true && it->second.userMax == it->second.clients.size()){
            std::string ERR_CHANNELISFULL = serverName + " 471 " + user.getNick() + " #" + channelName + " :Channel is full (+l)\r\n";
            send(user.getSocket(), ERR_CHANNELISFULL.c_str(), ERR_CHANNELISFULL.length(), 0);
            return 1;
        }

        if ((!channels[channelName].passKey.empty()) && channels[channelName].passKey != key)
        {
        //    std::cout << "'" << channels[channelName].passKey << "'  '" << key << std::endl;
            std::string ERR_BADCHANNELKEY = "ERR_BADCHANNELKEY :" + user.getNick() + " #" + channelName + ":Cannot join channel (+k)\r\n";
            send(user.getSocket(), ERR_BADCHANNELKEY.c_str(), ERR_BADCHANNELKEY.length(), 0);
            return 1;
        }
    }
    return 0;
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

void Channels::createNewChannel(const User& user, const std::string& channelName, bool& setOp){
    // Channel doesn't exist, create a new channel and add the user
    Channel newChannel;                           
    newChannel.clients.push_back(user);
    newChannel.operators.push_back(user);
    newChannel.topicMode = false;
    newChannel.inviteOnly = false;
    newChannel.userLimit = false;
    newChannel.userMax = 0;
    channels[channelName] = newChannel;
    setOp = true;
}

void Channels::channelOperators(const User& user, const std::string& channelName, bool& setOp){
    if (setOp == true) {
        //Set first user as operator
        std::string setOperator = serverName + " MODE #" + channelName + " +o " + user.getNick() + "\r\n";
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
        if (buffer.find(':') == std::string::npos){    //SEND TOPIC
            std::string RPL_TOPIC = serverName + " 332 " + user.getNick() + " #" + channelName + " :" + it->second.topic + "\r\n";
            send(user.getSocket(), RPL_TOPIC.c_str(), RPL_TOPIC.length(), 0);
        }
        else if (it->second.topicMode == false)    //CHECK TOPIC MODE AND SET TOPIC
            setTopic(user, channelName, arg);
        else if (it->second.topicMode == true && checkOperator(user, channelName)) //TOPIC MODE ON, CHECK OPERATOR AND SET TOPIC
            setTopic(user, channelName, arg);
    }
}

bool Channels::channelExist(std::string channelName)
{
    std::map<std::string, Channel >::iterator it = channels.find(channelName);
    if (it == channels.end())
        return false;
    else
        return true;
}