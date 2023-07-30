/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   message.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/19 15:05:46 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/30 20:49:25 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::messageHandler(User& user)
{
        std::memset(user.buffer, 0, sizeof(user.buffer));
		ssize_t bytesRead = recv(user.getSocket(), user.buffer, sizeof(user.buffer) - 1, 0);
		if (bytesRead < 0) {
			perror("Receive error");
			close(user.getSocket());
			return ;
		} else if (bytesRead == 0) {
            std::memset(user.buffer, 0, sizeof(user.buffer));
            user.msgBuffer.clear();
            std::string exit = ":Konversation terminated!\r\n";
            quit(&exit[0], user);
			return ;
		}
		printStringNoP(user.buffer, strlen(user.buffer));   
        user.msgBuffer += std::string(user.buffer);
        if (!std::strchr(user.buffer, '\n'))
            return ;
        commandHandler(user);
        user.msgBuffer.clear();
		std::memset(user.buffer, 0, sizeof(user.buffer));
}

void Server::commandHandler(User &user)
{
    std::string const & str = user.msgBuffer;
    if(str.length() >= 8 && !strncmp(str.c_str(), "JOIN #", 6))
			channels.multiChannelJoin(user, removeCRLF(&(str[5])));
    else if (str.length() >= 12 && !strncmp(str.c_str(), "PRIVMSG #", 9))
        channels.messageToChannel(user, removeCRLF(&(str[8])));
    else if (str.length() >= 11 && !strncmp(str.c_str(), "PRIVMSG ", 8))
        messageToPrivate(user, removeCRLF(&(str[8])));
    else if (str.length() >= 8 && !strncmp(str.c_str(), "PART ", 5))
    {
        std::string buf = removeCRLF(&(str[5]));
        std::string token = std::strtok(&(buf[0]), " ");
        if (buf.find(':') == buf.npos)
            channels.leaveChannel(user, &(token[1]), buf + " :Konversation terminated!\n");
        else
            channels.leaveChannel(user, &(token[1]), buf.substr(buf.find(':')));
    }
    else if (str.length() >= 7 && !strncmp(str.c_str(), "PING", 4))
    {
        std::string PONG = "PONG " + std::string(std::strtok(&user.msgBuffer[5], "\n")) + "\r\n";
        send(user.getSocket(), PONG.c_str(), PONG.length(), 0);
    }
    else if (str.length() >= 7 && !strncmp(str.c_str(), "QUIT ", 5))
        quit(&(user.msgBuffer[0]), user);
    else if (str.length() >= 7 && !strncmp(str.c_str(), "NICK ", 5))
        changeNick(&(str[5]), user, 0);
    else if (str.length() >= 12 && !strncmp(str.c_str(), "INVITE ", 7))
        invite(str.substr(7, str.length() - 1), user);
    else if (str.length() >= 10 && !strncmp(str.c_str(), "KICK #", 6))
        kick(str.substr(6, str.length() - 1), user);
    else if (str.length() >= 7 && !strncmp(str.c_str(), "TOPIC ", 6))
        channels.topic(user, removeCRLF(&str[0]));
    else if (str.length() >= 9 && !strncmp(str.c_str(), "MODE ", 5))
        modeHandler(user, str);
    else if (strncmp(str.c_str(), "WHO ", 4) && strncmp(str.c_str(), "USERHOST ", 9)){
        std::string ERR_UNKNOWNCOMMAND = serverName + " 421 " + user.getNick() + " " + removeCRLF(&str[0]) + " :Unknown command\r\n";
        send(user.getSocket(), ERR_UNKNOWNCOMMAND.c_str(), ERR_UNKNOWNCOMMAND.length(), 0);
    }
}

int Server::messageToPrivate(User& user, std::string buffer)
{
    std::stringstream ss(buffer);
    std::string name;
    ss >> name;
    std::string mex;
    std::getline(ss, mex, ' ');
    std::getline(ss, mex, '\n');

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
    std::string PRIVMSG = ":" + user.getNick() + "! PRIVMSG " + name + " " + mex + "\r\n";

    if (user.getNick() != name)
        send(clientSocket, PRIVMSG.c_str(), PRIVMSG.length(), 0);
    return 0;
}

void Server::clientDisconnected(const User& user){
    struct epoll_event event;
    event.events = 0;
    event.data.fd = user.getSocket();
    if (epoll_ctl(epollFd, EPOLL_CTL_DEL, user.getSocket(), &event) < 0)
        perror("Error removing client from epoll");
}

void Server::quit(char * buffer, User &user)
{
    std::string buf = buffer;
    std::vector <User> ::iterator it2 = clients.begin();
    std::string quitmsg = ":" + user.getNick() + "!" + user.getUser() + "@" + hostname + " QUIT " + buf.substr(buf.find(':')) + "\r\n";
    for (; it2 != clients.end(); ++it2) {
        send(it2->getSocket(), quitmsg.c_str(), quitmsg.length(), 0);}
    
	if (!strncmp(&buffer[6], "ragequit", 8)) // Server shutdown, only for valgrind test
		isServerRunning = false;
    
    /*for every channel in which the user is in
      remove the user from the list of channel clients 
      and channel operators(if no more operators assign op)*/
    std::vector <std::string> ::iterator it = user.getChannels().begin();
    for (; it != user.getChannels().end(); it++)
    {
        std::map<std::string, Channel> ::iterator cc = channels.getChannels().find(*it);
        if (cc != channels.getChannels().end())
        {
            std::vector<User> & chClients = cc->second.clients;
            std::vector<User> & chOper = cc->second.operators;
            
            // remove the user from the channel participants
            chClients.erase(std::remove(chClients.begin(), chClients.end(), user), chClients.end());
            
            //remove channel if there are no more users in the channel
            if  (chClients.size() == 0 ||
                (chClients.size() == 1 && findClientByName(chClients, "Mimmomodem") != -1))
                    channels.getChannels().erase(*it);
            else if (findClient(chOper, user) != -1)
            {   
                //remove op
                chOper.erase(std::remove(chOper.begin(), chOper.end(), user), chOper.end());
                //give op if no more op
                if (chOper.size() == 0 || (chOper.size() == 1 && findClientByName(chOper, "Mimmomodem") != -1) )
                {
                    std::vector<User>::iterator itUser = chClients.begin();
                    for (; itUser != chClients.end(); ++itUser)
                    {
                        if (itUser->getNick().compare("Mimmomodem") != 0)
                        {
                            chOper.push_back(*itUser);
                            std::string newOp = serverName + " MODE #" + *it + " +o " + itUser->getNick() + "\r\n";
                            channels.sendToAll(*it, newOp);
                            break ;
                        }
                    }
                }
            }
        }
    }

    // Remove socket client from epoll
    clientDisconnected(user);
    close(user.getSocket());
    user.setSocket(-1);

    //cleaning user data
    user.setNick("");
    user.setUser("");
    user.setIP("");
    memset(&(user.getAddr()), 0, sizeof(user.getAddr()));
    memset(&(user.getAddrLen()), 0, sizeof(user.getAddrLen()));
    user.getChannels().clear();
    memset(user.buffer, 0, sizeof(user.buffer));
	user.msgBuffer.clear();
    clientsConnected--;
}

int Server::changeNick(std::string buffer, User &user, int flag)
{
	if (buffer.find('\n') != buffer.npos)
		buffer = removeCRLF(&buffer[0]);
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
    
    std::string nickmsg2 = ":" + user.getNick() + "!" + user.getUser() + "@" + hostname + " NICK :" + buffer + "\r\n";
    
    if (flag == 0)
    {
        //search in channels and change nick
        std::vector<std::string>::iterator it2 = user.getChannels().begin();
        for (; it2 != user.getChannels().end(); it2++)
        {
            std::vector <User> ::iterator it3;
            std::vector <User>::iterator it4;
            it3 = std::find(channels.getChannels()[*it2].clients.begin(), channels.getChannels()[*it2].clients.end(), user);
            it4 = std::find(channels.getChannels()[*it2].operators.begin(), channels.getChannels()[*it2].operators.end(), user);
            if (it3 != channels.getChannels()[*it2].clients.end())
                it3->setNick(&(buffer[0]));
            if (it4 != channels.getChannels()[*it2].operators.end())
                it4->setNick(&(buffer[0]));
        }
    }
    
    user.setNick(&(buffer[0]));//set new nickname
    
    if (flag == 0)
    {    
        it = clients.begin();
        for (; it != clients.end(); it++)
            send(it->getSocket(), nickmsg2.c_str(), nickmsg2.length(), 0);
    }
    return 0;
}

void Server::invite(std::string buffer, User &user)
{
    buffer = removeCRLF(buffer.c_str());
    std::stringstream ss(buffer);
    std::string name;
    ss >> name;
    std::string channelName;
    std::getline(ss, channelName, '#');
    ss >> channelName;

    std::string ERR_NOSUCHCHANNEL = "ERR_NOSUCHCHANNEL :" + user.getNick() + " #" + channelName + "\r\n";//No such channel
    std::string ERR_NOTONCHANNEL = "ERR_NOTONCHANNEL :" + user.getNick() + " #" + channelName + "\r\n";//You're not on that channel
    std::string ERR_CHANOPRIVSNEEDED = "ERR_CHANOPRIVSNEEDED :" + user.getNick() + " #" + channelName + "\r\n";//You're not channel operator
    std::string ERR_USERONCHANNEL = "ERR_USERONCHANNEL :" + user.getNick() + " " + name + " #" + channelName + " :is already on channel\r\n";
    std::string RPL_INVITING = "RPL_INVITING: " + user.getNick() + " " + name + " #" + channelName + "\r\n";//to the issuer of the message 
    std::string INVITE = ":" + user.getNick() + "!" + user.getUser() + "@" + hostname + " INVITE " + name + " #" + channelName + "\r\n";//with the issuer as <source>, to target user

    if (channels.channelExist(channelName) == false) 
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
    std::stringstream ss(buffer);
    std::string channelName;
    ss >> channelName;
    std::string name;
    ss >> name;
    std::string mex;
    std::getline(ss, mex, ' ');
    std::getline(ss, mex, '\n');

//    std::cout << "'" << channelName << "' '" << name << "' '" << mex << "'" << std::endl;
    
    std::string ERR_NOSUCHCHANNEL = "ERR_NOSUCHCHANNEL :" + user.getNick() + " #" + channelName + "\r\n";
    std::string ERR_CHANOPRIVSNEEDED = "ERR_CHANOPRIVSNEEDED :" + user.getNick() + " #" + channelName + "\r\n";
    std::string ERR_NOTONCHANNEL = "ERR_NOTONCHANNEL :" + user.getNick() + " #" + channelName + "\r\n";
    std::string ERR_USERNOTINCHANNEL = "ERR_USERNOTINCHANNEL :" + user.getNick() +  " " + name + " #" + channelName + " :They aren't on that channel\r\n";
    std::string err;
    std::string KICK = ":" + user.getNick() + "!" + user.getUser() + "@" + hostname + " KICK #" + channelName + " " + name + " " + mex + "\r\n"; 

    if (channels.channelExist(channelName) == false) 
    {
        err = ERR_NOSUCHCHANNEL;
        send(user.getSocket(),  err.c_str(), err.length(), 0);
        return ;
    }
    std::vector <User> & chClients = ((channels.getChannels())[channelName]).clients;
//    std::cout << "Channel clients:" << std::endl;
//   printUsers(chClients);
    if (findClient(chClients, user) == -1)
    {
        err = ERR_NOTONCHANNEL;
        send(user.getSocket(),  err.c_str(), err.length(), 0);
        return ;
    }
    std::vector <User> & chOper = ((channels.getChannels())[channelName]).operators;
//    std::cout << "Channel Ops:" << std::endl;
//    printUsers(chOper);
    if (findClient(chOper, user) == -1)
    {
        err = ERR_CHANOPRIVSNEEDED;
        send(user.getSocket(),  err.c_str(), err.length(), 0);
        return ;// = buffer.substr(0, buffer.length() - 1);

    }
    if (findClientByName(chClients, name) == -1)
    {
        err = ERR_USERNOTINCHANNEL;
        send(user.getSocket(),  err.c_str(), err.length(), 0);
        return ;
    }
    channels.sendToAll(channelName, KICK);
 
    User & target = chClients[findClientByName(chClients, name)];
    //remove op
    if (findClientByName(chOper, name) != -1)
        chOper.erase(std::remove(chOper.begin(), chOper.end(), target), chOper.end());
    // remove the user from the channel participants
    chClients.erase(std::remove(chClients.begin(), chClients.end(), target), chClients.end());
    // Update user channel list
    target.getChannels().erase(std::remove(target.getChannels().begin(), target.getChannels().end(), channelName), target.getChannels().end());
    
    //(if you kicked yourserlf)
    //remove channel if there are no more users in the channel
    if  (chClients.size() == 0 ||    
        (chClients.size() == 1 && findClientByName(chClients, "Mimmomodem") != -1))
            channels.getChannels().erase(channelName); 
    //if no more op, assign op
    else if (chOper.size() == 0 || (chOper.size() == 1 && findClientByName(chOper, "Mimmomodem") != -1) )
    {
        std::vector<User>::iterator itUser = chClients.begin();
        for (; itUser != chClients.end(); ++itUser)
        {
            if (itUser->getNick().compare("Mimmomodem") != 0)
            {
                chOper.push_back(*itUser);
                std::string newOp = serverName + " MODE #" + channelName + " +o " + itUser->getNick() + "\r\n";
                channels.sendToAll(channelName, newOp);
                break ;
            }
        }
    }
}