/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   message.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/19 15:05:46 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/24 15:26:07 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::messageHandler(User& user)
{
		char buffer[1024];
        std::memset(buffer, 0, sizeof(buffer));
		ssize_t bytesRead = recv(user.getSocket(), buffer, sizeof(buffer) - 1, 0);
		if (bytesRead < 0) {
			perror("Receive error");
			close(user.getSocket());
			return ;
		} else if (bytesRead == 0) {
			close(user.getSocket());
			user.setSocket(-1);
			return ;
		}
		printStringNoP(buffer, static_cast<std::size_t>(bytesRead));        
        msgBuffer += std::string(buffer);
        if (!std::strchr(buffer, '\n'))
            return ;
        commandHandler(user);
        msgBuffer.clear();
		std::memset(buffer, 0, sizeof(buffer));
}

void Server::commandHandler(User &user)
{
    if(!strncmp(msgBuffer.c_str(), "JOIN #", 6))
			channels.joinChannel(user, removeCRLF(&(msgBuffer[6])));
    else if (!strncmp(msgBuffer.c_str(), "PRIVMSG #", 9))
        channels.messageToChannel(user, removeCRLF(&(msgBuffer[8])));
    else if (!strncmp(msgBuffer.c_str(), "PRIVMSG ", 8))
        messageToPrivate(user, removeCRLF(&(msgBuffer[8])));
    else if (!strncmp(msgBuffer.c_str(), "PART ", 5))
    {
        std::string buf = removeCRLF(&(msgBuffer[5]));
        std::string token = std::strtok(&(buf[0]), " ");
        channels.leaveChannel(user, &(token[1]), buf.substr(buf.find(':')));
    }
    else if (!strncmp(msgBuffer.c_str(), "PING", 4))
    {
        std::string PONG = "PONG " + std::string(std::strtok(&msgBuffer[5], "\n")) + "\r\n";
        send(user.getSocket(), PONG.c_str(), PONG.length(), 0);
    }
    else if (!strncmp(msgBuffer.c_str(), "QUIT ", 5))
        quit(&(msgBuffer[0]), user);
    else if (!strncmp(msgBuffer.c_str(), "NICK ", 5))
        changeNick(&(msgBuffer[5]), user, 0);
    else if (!strncmp(msgBuffer.c_str(), "INVITE ", 7))
        invite(msgBuffer.substr(7, msgBuffer.length() - 1), user);
    else if (!strncmp(msgBuffer.c_str(), "KICK ", 5))
        kick(msgBuffer.substr(5, msgBuffer.length() - 1), user);
    else if (!strncmp(msgBuffer.c_str(), "TOPIC ", 6))
        channels.topic(user, removeCRLF(&msgBuffer[0]));
    else if (!strncmp(msgBuffer.c_str(), "MODE ", 5))
    {
        std::string mode = findMode(msgBuffer);
        if (mode.empty())
            return ;
        else if (mode.find('o') != std::string::npos)
            channels.setModeOperator(user, msgBuffer, mode);
        else if (mode.find('t') != std::string::npos)
            channels.setModeTopic(user, std::strtok(&msgBuffer[6], " "), mode);
    }
  /*   else if (strncmp(msgBuffer.c_str(), "WHO ", 4) && strncmp(msgBuffer.c_str(), "USERHOST ", 9)){
        std::string ERR_UNKNOWNCOMMAND = serverName + " 421 " + user.getNick() + " " + removeCRLF(&msgBuffer[0]) + " :Unknown command\r\n";
        printStringNoP(ERR_UNKNOWNCOMMAND.c_str(), ERR_UNKNOWNCOMMAND.length());
        send(user.getSocket(), ERR_UNKNOWNCOMMAND.c_str(), ERR_UNKNOWNCOMMAND.length(), 0);
    } */
}

int Server::messageToPrivate(User& user, std::string buffer)
{
    std::string name = buffer.substr(0, buffer.find(' '));
    std::string mex = buffer.substr(name.length() + 1, std::string::npos);

    int clientSocket = -1;
    std::vector<User> ::iterator it = clients.begin();
    for (; it != clients.end(); it++)
    {
        if (it->getNick() == name)
        {
            clientSocket = it->getSocket();
            break;
        }
    }
    std::string PRIVMSG = ":" + user.getNick() + "! PRIVMSG " + name + " " + mex.substr(0, mex.length()) + "\r\n";

    if (user.getNick() != name)
        send(clientSocket, PRIVMSG.c_str(), PRIVMSG.length(), 0);
    return 0;
}

void Server::quit(char * buffer, User &user)
{
    std::string buf = buffer;
    std::vector <User> ::iterator it2 = clients.begin();
    std::string quitmsg = ":" + user.getNick() + "!" + user.getUser() + "@" + hostname + " QUIT " + buf.substr(buf.find(':')) + "\r\n";
    for (; it2 != clients.end(); ++it2)
        send(it2->getSocket(), quitmsg.c_str(), quitmsg.length(), 0);

    close(user.getSocket());
    user.setSocket(-1);
	if (!strncmp(&buffer[6], "ragequit", 8))
		isServerRunning = false;
    user.setNick("");
    user.setUser("");
    user.setIP("");
}

int Server::changeNick(std::string buffer, User &user, int flag)
{
    buffer = buffer.substr(0, buffer.length() - 1);
    std::vector <User> ::iterator it = clients.begin();
    for (; it != clients.end(); it++)
    {
        if (buffer == it->getNick())
        {
            //send Nickname already taken and than return 
            std::string ERR_NICKNAMEINUSE = serverName + " 433 * " + it->getNick() + " :Nickname is already in use\r\n";
            send(user.getSocket(), ERR_NICKNAMEINUSE.c_str(),ERR_NICKNAMEINUSE.length(), 0);
            return 1;
        }
    }
    
    std::string nickmsg = ":" + user.getNick() + " NICK " + buffer + "\r\n";
    std::string nickmsg2 = ":" + user.getNick() + "!" + user.getUser() + "@" + hostname + " NICK :" + buffer + "\r\n";
    
    if (flag == 0)
    {
        std::cout << nickmsg;
        //search in channels and change nick
        std::vector<std::string>::iterator it2 = user.getChannels().begin();
        for (; it2 != user.getChannels().end(); it2++)
        {
            std::vector <User> ::iterator it3;
            it3 = std::find(channels.getChannels()[*it2].clients.begin(), channels.getChannels()[*it2].clients.end(), user);
            if (it3 != channels.getChannels()[*it2].clients.end())
                it3->setNick(&(buffer[0]));
        }
    }
    
    user.setNick(&(buffer[0]));//set new nickname
    
    if (flag == 0)
    {    
        it = clients.begin();
        for (; it != clients.end(); it++)
        {
            send(it->getSocket(), nickmsg.c_str(), nickmsg.length(), 0);
            send(it->getSocket(), nickmsg2.c_str(), nickmsg2.length(), 0);
        }
    }
    return 0;
}

bool Server::channelExist2(std::string channelName)
{
    std::map<std::string, Channel >::iterator it = channels.getChannels().find(channelName);
    if (it == channels.getChannels().end())
        return false;
    else
        return true;
}

int Server::findClient(std::vector <User> chClients, User user)
{
    std::vector <User> ::iterator it = chClients.begin();
    for (int i = 0; it != chClients.end(); it++)
    {;
        if (*it == user)
            return i;
        i++;
    }
    return -1;
}

int Server::findClientByName(std::vector <User> chClients, std::string name)
{
    std::vector <User> ::iterator it = chClients.begin();
    for (int i = 0; it != chClients.end(); it++)
    {
        if (it->getNick() == name)
            return i;
        i++;
    }
    return -1;
}

void Server::invite(std::string buffer, User &user)
{
    std::string name = buffer.substr(0, buffer.find(' '));
    std::string channelName = buffer.substr(name.length() + 2, buffer.length() - name.length() - 3);
    std::string ERR_NOSUCHCHANNEL = "ERR_NOSUCHCHANNEL :" + user.getNick() + " #" + channelName + "\r\n";//No such channel
    std::string ERR_NOTONCHANNEL = "ERR_NOTONCHANNEL :" + user.getNick() + " #" + channelName + "\r\n";//You're not on that channel
    std::string ERR_CHANOPRIVSNEEDED = "ERR_CHANOPRIVSNEEDED :" + user.getNick() + " #" + channelName + "\r\n";//You're not channel operator
    std::string ERR_USERONCHANNEL = "ERR_USERONCHANNEL :" + user.getNick() + " " + name + " #" + channelName + " :is already on channel\r\n";
    std::string RPL_INVITING = "RPL_INVITING: " + user.getNick() + " " + name + " #" + channelName + "\r\n";//to the issuer of the message 
    std::string INVITE = ":" + user.getNick() + "!" + user.getUser() + "@" + hostname + " INVITE " + name + " #" + channelName + "\r\n";//with the issuer as <source>, to target user

    if (channelExist2(channelName) == false) 
    {
        send(user.getSocket(), ERR_NOSUCHCHANNEL.c_str(), ERR_NOSUCHCHANNEL.length(), 0);
        return ;
    }
    
    std::vector <User> chClients = ((channels.getChannels())[channelName]).clients;
    if (findClient(chClients, user) == -1)
    {
        send(user.getSocket(), ERR_NOTONCHANNEL.c_str(), ERR_NOTONCHANNEL.length(), 0);
        return ;
    }

    std::vector <User> chOper = ((channels.getChannels())[channelName]).operators;
    if ((channels.getChannels())[channelName].inviteOnly == true)
    {
        if (findClient(chOper, user) == -1)
        {
            send(user.getSocket(), ERR_CHANOPRIVSNEEDED.c_str(), ERR_CHANOPRIVSNEEDED.length(), 0);
            return ;
        }
        else
            ((channels.getChannels())[channelName]).invitelist.insert(name);
    }

    if (findClientByName(chClients, name) != -1)
    {
        send(user.getSocket(), ERR_USERONCHANNEL.c_str(), ERR_USERONCHANNEL.length(), 0);
        return ;
    }

    User target = clients[findClientByName(clients, name)];
    send(user.getSocket(), RPL_INVITING.c_str(), RPL_INVITING.length(), 0);
    send(target.getSocket(), INVITE.c_str(), INVITE.length(), 0);
}

void Server::kick(std::string buffer, User &user)
{
    std::string channelName = buffer.substr(1, buffer.find(' ') - 1);
    buffer = buffer.substr(channelName.length() + 2, std::string::npos);
    std::string name = buffer.substr( 0, buffer.find(' '));
    buffer = buffer.substr(name.length() + 2, std::string::npos);
    std::string mex = buffer.substr(0, buffer.length() - 1);
    
    std::string ERR_NOSUCHCHANNEL = "ERR_NOSUCHCHANNEL :" + user.getNick() + " #" + channelName + "\r\n";
    std::string ERR_CHANOPRIVSNEEDED = "ERR_CHANOPRIVSNEEDED :" + user.getNick() + " #" + channelName + "\r\n";
    std::string ERR_NOTONCHANNEL = "ERR_NOTONCHANNEL :" + user.getNick() + " #" + channelName + "\r\n";
    std::string ERR_USERNOTINCHANNEL = "ERR_USERNOTINCHANNEL :" + user.getNick() +  " " + name + " #" + channelName + " :They aren't on that channel\r\n";
    std::string err;
    //:WiZ!jto@tolsun.oulu.fi KICK #Finnish John
    std::string KICK = ":" + user.getNick() + "!" + user.getUser() + "@" + hostname + " KICK #" + channelName + " " + name + " :" + mex + "\r\n"; 

    if (channelExist2(channelName) == false) 
    {
        err = ERR_NOSUCHCHANNEL;
        send(user.getSocket(),  err.c_str(), err.length(), 0);
        return ;
    }
    std::vector <User> chClients = ((channels.getChannels())[channelName]).clients;
    if (findClient(chClients, user) == -1)
    {
        err = ERR_NOTONCHANNEL;
        send(user.getSocket(),  err.c_str(), err.length(), 0);
        return ;
    }
    std::vector <User> chOper = ((channels.getChannels())[channelName]).operators;
    if (findClient(chOper, user) == -1)
    {
        err = ERR_CHANOPRIVSNEEDED;
        send(user.getSocket(),  err.c_str(), err.length(), 0);
        return ;
    }
    if (findClientByName(chClients, name) == -1)
    {
        err = ERR_USERNOTINCHANNEL;
        send(user.getSocket(),  err.c_str(), err.length(), 0);
        return ;
    }
    channels.sendToAll(channelName, KICK);
    
    // Channel exists, remove the user from the channel participants
    std::vector<User> & channelusers = ((channels.getChannels())[channelName]).clients;
    channelusers.erase(std::remove(channelusers.begin(), channelusers.end(), user), channelusers.end());
    // Update user channel list
    user.getChannels().erase(std::remove(user.getChannels().begin(), user.getChannels().end(), channelName), user.getChannels().end());
}