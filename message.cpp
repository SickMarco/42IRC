/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   message.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/19 15:05:46 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/22 16:07:49 by mbozzi           ###   ########.fr       */
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
    {//INVITE mbozzi #hh\xa
    }
    else if (!strncmp(msgBuffer.c_str(), "KICK ", 5))
    {//KICK #hh mbozzi :perchessi\xa
    }
    else if (!strncmp(msgBuffer.c_str(), "TOPIC ", 6))
        channels.topic(user, removeCRLF(&msgBuffer[0]));
    else if (!strncmp(msgBuffer.c_str(), "MODE ", 5))
    {//MODE #hh +o mabaffo\xa
    }
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