#include "Channels.hpp"

std::vector<std::string> Channels::split(std::string s, char delimiter)
{
    std::vector<std::string> splitted;
    std::string token;
    std::istringstream tokenStream(s);

    while (std::getline(tokenStream, token, delimiter))
        splitted.push_back(token);
    return splitted;
}

void Channels::multiChannelJoin(User& user, std::string buffer)
{
    std::string channelNames = buffer.substr(0, buffer.find(' '));
    std::string keys;
    if (buffer.find(' ') != buffer.npos)
        keys = buffer.substr(channelNames.length() + 1, buffer.npos);
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
        joinChannel(user, &(name[1]), key);
        if (it2 != chKeys.end())
            it2++;
    }
}

int Channels::joinChannel(User& user, std::string channelName, std::string key)
{
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

    bool setOp = false;
    if (channelName.find(' ') != channelName.npos)
        channelName = std::strtok(&channelName[0], " ");
    if (!channelExist(user, channelName))
        createNewChannel(user, channelName, setOp);
    user.getChannels().push_back(channelName);
    joinMessageSequence(user, channelName);
    channelOperators(user, channelName, setOp);
    return 0;
}

bool Channels::channelExist(User& user, std::string channelName)
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
        return true;
    }
    return false;
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
    send(user.getSocket(), RPL_NAMREPLY.c_str(), RPL_NAMREPLY.length(), 0);
    send(user.getSocket(), RPL_ENDOFNAMES.c_str(), RPL_ENDOFNAMES.length(), 0);
}
