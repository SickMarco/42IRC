/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   message.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/19 15:05:46 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/19 15:35:44 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::messageHandler(User& user){
		char buffer[1024];
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
		buffer[bytesRead] = '\0';
		printStringNoP(buffer, static_cast<std::size_t>(bytesRead));
		/* if(!strncmp(buffer, "NICK", 4))
			user.changeNickname(buffer); */
	 	if(!strncmp(buffer, "JOIN #", 6))
			joinChannel(removeCRLF(&(buffer[6])), user);
        else if (!strncmp(buffer, "PRIVMSG #", 9))
            messageToChannel(user, removeCRLF(&(buffer[8])));
        else if (!strncmp(buffer, "PRIVMSG ", 8))
            messageToPrivate(user, removeCRLF(&(buffer[8])));
        else if (!strncmp(buffer, "PART ", 5))
        {
            //std::cout << "BUFFER: " << buffer << std::endl;
            std::string buf = removeCRLF(&(buffer[5]));
            std::string token = std::strtok(&(buf[0]), " ");
            leaveChannel(&(token[1]), user, buf.substr(buf.find(':')));
        }
        else if (!strncmp(buffer, "PING", 4))
        {
            std::string PONG = "PONG " + std::string(std::strtok(&buffer[5], "\n")) + "\r\n";
            send(user.getSocket(), PONG.c_str(), PONG.length(), 0);
        }
        else if (!strncmp(buffer, "QUIT ", 5))
            quit(buffer, user);
        else if (!strncmp(buffer, "NICK ", 5))
            changeNick(&(buffer[5]), user);
        else if (!strncmp(buffer, "USER", 4))
        {

        }
		std::memset(buffer, 0, sizeof(buffer));
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
    std::string PRIVMSG = ":" + user.getNick() + " PRIVMSG " + name + " " + mex.substr(0, mex.length()) + "\r\n";

    bool sentToDest = false;
    if (user.getNick() != name) {
        send(clientSocket, PRIVMSG.c_str(), PRIVMSG.length(), 0);
        sentToDest = true;
    }
    if (!sentToDest)
        send(user.getSocket(), PRIVMSG.c_str(), PRIVMSG.length(), 0);
    return 0;
}

int Server::messageToChannel(User& user, std::string buffer)
{
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

void Server::quit(char * buffer, User &user)
{
    std::string buf = buffer;
    std::vector <User> ::iterator it2 = clients.begin();
    std::string quitmsg = ":" + user.getNick() + "!"+ user.getUser() + "@" + hostname + " QUIT " + buf.substr(buf.find(':')) + "\r\n";
    for (; it2 != clients.end(); ++it2)
        send(it2->getSocket(), quitmsg.c_str(), quitmsg.length(), 0);

    close(user.getSocket());
    user.setSocket(-1);
	if (!strncmp(&buffer[6], "ragequit", 8))
		isServerRunning = false;
    std::vector <std::string> ::iterator it = user.channelsJoined.begin();
    std::vector <User> ccv;
    for (; it != user.channelsJoined.end(); ++it)
    {
        ccv =  channels[*it].clients;
        ccv.erase(std::remove(ccv.begin(), ccv.end(), user), ccv.end());
    }
    clients.erase(std::remove(clients.begin(), clients.end(), user), clients.end());
}

void Server::changeNick(std::string buffer, User &user)
{
    buffer = buffer.substr(0, buffer.length() - 1);
    std::vector <User> ::iterator it = clients.begin();
    for (; it != clients.end(); it++)
    {
        if (buffer == it->getNick())
            return ;//send Nickname already taken and than return 
            //:oldnick!user@host NICK :newnick
    }

    std::string nickmsg = ":" + user.getNick() + " NICK " + buffer + "\r\n";
    std::string nickmsg2 = ":" + user.getNick() + "!" + user.getUser() + "@" + hostname + " NICK :" + buffer + "\r\n";
    std::cout << nickmsg;
    //search in channels and change nick
    std::vector <std::string> ::iterator it2 = user.channelsJoined.begin();
    for (; it2 != user.channelsJoined.end(); it2++)
    {
        std::vector <User> ::iterator it3;
        it3 = std::find(channels[*it2].clients.begin(), channels[*it2].clients.end(), user);
        if (it3 != channels[*it2].clients.end())
            it3->setNick(&(buffer[0]));
    }

    user.setNick(&(buffer[0]));//set new nickname
    it = clients.begin();
    for (; it != clients.end(); it++)
    {
        send(it->getSocket(), nickmsg.c_str(), nickmsg.length(), 0);
        send(it->getSocket(), nickmsg2.c_str(), nickmsg2.length(), 0);
    }
}