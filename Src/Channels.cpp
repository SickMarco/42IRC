/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channels.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/22 10:58:42 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/31 19:58:38 by mbozzi           ###   ########.fr       */
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
            send(it->second.clients[i].getSocket(), message.c_str(), message.length(), sndFlags);
        }
    }
}

int Channels::messageToChannel(const User& user, std::string buffer)
{
    std::stringstream ss(&(buffer[1]));
    std::string channelName;
    ss >> channelName;
    std::string mex;
    std::getline(ss, mex, ' ');
    std::getline(ss, mex, '\n');
    std::string ERR_NOTONCHANNEL = "ERR_NOTONCHANNEL :" + user.getNick() + " #" + channelName + "\r\n";
    if (mex[0] != ':')
    {
        unknownCommand(user, buffer);
        return 0;
    }
    std::map<std::string, Channel >::iterator it = channels.find(channelName);
    if (it == channels.end()){
		std::string ERR_NOSUCHNICK = serverName + " 401 " + user.getNick() + " " + channelName + " :No such channel\r\n";
		send(user.getSocket(), ERR_NOSUCHNICK.c_str(), ERR_NOSUCHNICK.length(), sndFlags);
		return -1;
	}
    
    if (findClientByName(channels[channelName].clients, user.getNick()) == -1)
    {
        send(user.getSocket(),  ERR_NOTONCHANNEL.c_str(), ERR_NOTONCHANNEL.length(), sndFlags);
        return 0;
    }

    if (buffer.find("!bot ") != buffer.npos && findClientByName(channels[channelName].clients ,"Mimmomodem") != -1){
		botCommand(user, channelName, buffer);
		return 1;
	}

    if (it != channels.end())
    {
        if ((channels[channelName]).censorship == true)
            censorshipBot(mex);
        std::vector<User> channelClients = it->second.clients;
        std::string privmsg = ":" + user.getNick() + " PRIVMSG #" + channelName + " " + mex + "\r\n";

        for (size_t i = 0; i < channelClients.size(); ++i) {
            if (channelClients[i].getSocket() != -1 && channelClients[i].getSocket() != user.getSocket())
                send(channelClients[i].getSocket(), privmsg.c_str(), privmsg.length(), sndFlags);
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
            send(user.getSocket(), ERR_INVITEONLYCHAN.c_str(), ERR_INVITEONLYCHAN.length(), sndFlags);
            return 1;
        }
        if (it->second.userLimit == true && it->second.userMax <= it->second.clients.size() && findClientByName(it->second.clients, user.getNick()) == -1){
            std::string ERR_CHANNELISFULL = serverName + " 471 " + user.getNick() + " #" + channelName + " :Channel is full (+l)\r\n";
            send(user.getSocket(), ERR_CHANNELISFULL.c_str(), ERR_CHANNELISFULL.length(), sndFlags);
            return 1;
        }
        if ((!channels[channelName].passKey.empty()) && channels[channelName].passKey != key)
        {
            std::string ERR_BADCHANNELKEY = "ERR_BADCHANNELKEY :" + user.getNick() + " #" + channelName + ":Cannot join channel (+k)\r\n";
            send(user.getSocket(), ERR_BADCHANNELKEY.c_str(), ERR_BADCHANNELKEY.length(), sndFlags);
            return 1;
        }
    }
    return 0;
}

void Channels::leaveChannel(User& user, std::string channelName, std::string message)
{
    // Check if the channel exists
    std::map<std::string, Channel >::iterator it = channels.find(channelName);
    if (it == channels.end()){
		std::string ERR_NOSUCHNICK = serverName + " 401 " + user.getNick() + " " + channelName + " :No such channel\r\n";
		send(user.getSocket(), ERR_NOSUCHNICK.c_str(), ERR_NOSUCHNICK.length(), sndFlags);
		return ;
	}
    (void )message;
    if (it != channels.end())
    {
        std::string PART = ":" + user.getNick() + "!" + user.getUser() + "@" + hostname +" PART :#" + channelName + "\r\n";
        //Notify all channel users
        std::vector<User>::iterator itc = it->second.clients.begin();
        for (; itc != it->second.clients.end(); ++itc)
            send(itc->getSocket(), PART.c_str(), PART.length(), sndFlags);
        //remove the user from the channel participants
        std::vector<User> & channelusers = it->second.clients;
        channelusers.erase(std::remove(channelusers.begin(), channelusers.end(), user), channelusers.end());
        //remove op
        std::vector<User> & channelops = it->second.operators;
        if (findClientByName(channelops, user.getNick()) != -1)
            channelops.erase(std::remove(channelops.begin(), channelops.end(), user), channelops.end());
        // Update user channel list
        std::vector<std::string> & uChans = user.getChannels();
        uChans.erase(std::remove(uChans.begin(), uChans.end(), channelName), uChans.end());
        
        if  (channelusers.size() == 0 ||    //remove channel if there are no more users in the channel
            (channelusers.size() == 1 && findClientByName(channelusers, "Mimmomodem") != -1))
            channels.erase(channelName);
        else if  (channelops.size() == 0 ||  //give op if there are no more op in the channel
            (channelops.size() == 1 && findClientByName(channelops, "Mimmomodem") != -1))
        {
            std::vector<User>::iterator itUser = channelusers.begin();
            for (; itUser != channelusers.end(); ++itUser)
            {
                if (itUser->getNick().compare("Mimmomodem") != 0)
                {
                    channelops.push_back(*itUser);
                    std::string newOp = serverName + " MODE #" + channelName + " +o " + itUser->getNick() + "\r\n";
                    sendToAll(channelName, newOp);
                    break ;
                }
            }
        }
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
    newChannel.censorship = true;
    channels[channelName] = newChannel;
    setOp = true;
}

void Channels::channelOperators(const User& user, const std::string& channelName, bool& setOp){
    if (setOp == true) {
        //Set first user as operator
        std::string setOperator = serverName + " MODE #" + channelName + " +o " + user.getNick() + "\r\n";
        send(user.getSocket(), setOperator.c_str(), setOperator.length(), sndFlags);
		User bot;
		bot.setNick("Mimmomodem");
		channels[channelName].clients.push_back(bot);
        std::string BOT = ":Mimmomodem!Mimmomodem@bot JOIN #" + channelName + "\r\n";
        send(user.getSocket(), BOT.c_str(), BOT.length(), sndFlags);
    }
    else {
        //Notify all user of channels operators
        std::map<std::string, Channel >::iterator it = channels.find(channelName);
        std::vector<User>::iterator opIt = it->second.operators.begin();
        for (; opIt != it->second.operators.end(); ++opIt) {
            std::string sendOperator = serverName + " MODE #" + channelName + " +o " + opIt->getNick() + "\r\n";
            send(user.getSocket(), sendOperator.c_str(), sendOperator.length(), sndFlags);
        }
    }
    std::string botAnnounce = ":Mimmomodem PRIVMSG #" + channelName + " :Hi " + user.getNick() + " welcome to the channel #" + channelName + " [!bot help for command list]\r\n";
    send(user.getSocket(), botAnnounce.c_str(), botAnnounce.length(), sndFlags);
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
        send(user.getSocket(), ERR_CHANOPRIVSNEEDED.c_str(), ERR_CHANOPRIVSNEEDED.length(), sndFlags);
    }
    return false;
}

void Channels::setTopic(const User& user, const std::string& channelName, std::string& arg){
    arg.erase(std::remove(arg.begin(), arg.end(), ':'), arg.end());
    std::map<std::string, Channel>::iterator it = channels.find(channelName);
    if (it != channels.end()) {
        it->second.topic = arg;
        std::string RPL_TOPICSET = serverName + " 332 " + user.getNick() + " #" + channelName + " :" + arg + "\r\n";
        sendToAll(channelName, RPL_TOPICSET);
    }
}

void Channels::topic(const User& user, std::string buffer){
    std::string cmd, channelName, arg;

    if (strchr(buffer.c_str(), '#') != NULL)
        buffer.erase(std::remove(buffer.begin(), buffer.end(), '#'), buffer.end());

    std::istringstream iss(buffer);
    iss >> cmd >> channelName >> arg;

    std::map<std::string, Channel>::iterator it = channels.find(channelName);
    if (it != channels.end()){
        if (buffer.find(':') == std::string::npos){    //SEND TOPIC
            std::string RPL_TOPIC = serverName + " 332 " + user.getNick() + " #" + channelName + " :" + it->second.topic + "\r\n";
            send(user.getSocket(), RPL_TOPIC.c_str(), RPL_TOPIC.length(), sndFlags);
        }
        else if (it->second.topicMode == false)    //CHECK TOPIC MODE AND SET TOPIC
            setTopic(user, channelName, arg);
        else if (it->second.topicMode == true && checkOperator(user, channelName)) //TOPIC MODE ON, CHECK OPERATOR AND SET TOPIC
            setTopic(user, channelName, arg);
    }
    else {
        std::string ERR_NOSUCHNICK = serverName + " 401 " + user.getNick() + " " + channelName + " :No such channel\r\n";
		send(user.getSocket(), ERR_NOSUCHNICK.c_str(), ERR_NOSUCHNICK.length(), sndFlags);
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

void Channels::censorshipBot(std::string &mex)
{
    std::ifstream inputFile("./Src/.bannedWords.txt");
    
    if (!inputFile)
        return ;
        

    std::string word;
    std::vector <std::string> bannedWords;
    while (inputFile >> word)
        bannedWords.push_back(word);

    std::vector <std::string> ::iterator vit = bannedWords.begin();
    for (size_t i = 0; vit != bannedWords.end(); vit++, i++)
    {
        std::size_t found = mex.find(bannedWords[i]);
        if (found != std::string::npos)
        {
            for (size_t j = 0; j < bannedWords[i].size(); j++)
            {
                mex[found] = '*';
                found++;
            }
        }
    }
}

void Channels::unknownCommand(const User& user, const std::string& cmd)
{
    std::string ERR_UNKNOWNCOMMAND = serverName + " 421 " + user.getNick() + " " + removeCRLF(cmd.c_str()) + " :Unknown command\r\n";
    send(user.getSocket(), ERR_UNKNOWNCOMMAND.c_str(), ERR_UNKNOWNCOMMAND.length(), sndFlags);
}