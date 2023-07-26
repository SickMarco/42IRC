/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/11 17:27:53 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/26 17:41:59 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(const int& p, const std::string& pass) : serverName(":ludri"), serverPassword("ktm"), userPassword(pass), port(p) , isServerRunning(true){
	if (port != SERVER_PORT)
		throw std::runtime_error("Error: Wrong port!");
	if(userPassword != serverPassword)
		throw std::runtime_error("Error: Wrong password!");
    clients.resize(MAX_CLIENTS);
    skt.getMyIP(hostname, IP);
	skt.socketInit(IP);
	skt.binding();
    channels.init(serverName, hostname);
    std::cout << "SERVER IP: \033[0;35m" << IP << "\033[0;37m" << std::endl;
}

Server::~Server(){
	close(skt.getSocket());
}

void Server::welcomeMsg(const User& user){
    std::string RPL_WELCOME = serverName + " 001 " + user.getNick() + " :Welcome to the Internet Relay Network " + user.getNick() + "\r\n";
    std::string RPL_YOURHOST = serverName + " 002 " + user.getNick() + " :Hosted by Frat Carnal, running version 0.42\r\n";
    std::string RPL_CREATED = serverName + " 003 " + user.getNick() + " :This server was created on 2023/07/19\r\n";
    std::string RPL_MOTD = serverName + " 372 " + user.getNick() + " :- Welcome to the most ludro IRC server ever -\r\n";
    std::string RPL_ENDOFMOTD = serverName + " 376 " + user.getNick() + " :End of MOTD command\r\n";
    send(user.getSocket(), RPL_WELCOME.c_str(), RPL_WELCOME.length(), 0);
    send(user.getSocket(), RPL_YOURHOST.c_str(), RPL_YOURHOST.length(), 0);
    send(user.getSocket(), RPL_CREATED.c_str(), RPL_CREATED.length(), 0);
    send(user.getSocket(), RPL_MOTD.c_str(), RPL_MOTD.length(), 0);
    send(user.getSocket(), RPL_ENDOFMOTD.c_str(), RPL_ENDOFMOTD.length(), 0);
}

bool Server::checkPassword(User& user, const std::string& PASS){
    if (PASS != serverPassword && !serverPassword.empty())
    {
        std::string ERR_PASSWDMISMATCH = serverName + " ERR_PASSWDMISMATCH :Password incorrect.\r\n";
        send(user.getSocket(), ERR_PASSWDMISMATCH.c_str(), ERR_PASSWDMISMATCH.length(), 0);
        std::string quitmsg = "ERROR :Closing Link: " + IP + " (Connection refused by server)\r\n";
        send(user.getSocket(), quitmsg.c_str(), quitmsg.length(), 0);
        
        close(user.getSocket());
        user.setSocket(-1);
        user.setNick("");
        user.setUser("");
        user.setIP("");
        return false;
    }
    return true;
}

bool Server::setNewUser(User& user, const std::string& newClientMessage){
	bool userAlreadExist = false;
	ssize_t bytesRead;
	std::string command, param;
	std::stringstream ss(newClientMessage);
	char delim;
	char buffer[1024];

    while (std::getline(ss, command, '\n')){
        ss >> command;
        if (command.find("PASS") != command.npos){
			ss >> delim >> param;
            if (checkPassword(user, param) == false)
                return false;
		}
        else if (command.find("NICK") != command.npos){
			ss >> param;
			userAlreadExist = changeNick(param, user, 1);
		}
        else if (command.find("USER") != command.npos){
			ss.ignore(1, ' ');
			std::getline(ss, param, ' ');
			user.setUser(param);
		}
    }
	while (userAlreadExist == true){
		bytesRead = recv(user.getSocket(), buffer, sizeof(buffer) - 1, 0);
        buffer[bytesRead] = '\0';
		printStringNoP(buffer, strlen(buffer));
		if (!strncmp(buffer, "NICK ", 5))
			userAlreadExist = changeNick(removeCRLF(&buffer[5]), user, 1);
		memset(buffer, 0, 1024);
	}
	return true;
}

int Server::newClientConnected(User& user)
{
	
    ssize_t bytesRead;
    std::string newClientMessage, command, param;
	user.setIP(IP);
	char buffer[1024];
    while (1)
    {
        bytesRead = recv(user.getSocket(), buffer, sizeof(buffer) - 1, 0);
        buffer[bytesRead] = '\0';
        newClientMessage += buffer;
        memset(buffer, 0, 1024);
        if (newClientMessage.find("\nPASS") != newClientMessage.npos &&
            newClientMessage.find("\nNICK") != newClientMessage.npos &&
            newClientMessage.find("\nUSER") != newClientMessage.npos)
                break;
    }
	printStringNoP(newClientMessage.c_str(), newClientMessage.length());
	if (setNewUser(user, newClientMessage) == false)
		return 1;
    welcomeMsg(user);
    return 0;
}

void Server::newClientHandler(struct pollfd* fds, int& numClients) {
    if (fds[0].revents & POLLIN)
    {
        int clientSocket = accept(skt.getSocket(), NULL, NULL);
        if (clientSocket < 0){
            perror("Accept error");
            return;
        }
        // Aggiungi il nuovo client all'array
        if (numClients < MAX_CLIENTS) {
            int i;
            for (i = 0; i < MAX_CLIENTS; ++i) {
                if (clients[i].getSocket() == -1) {
                    clients[i].setSocket(clientSocket);
                    fds[i + 1].fd = clientSocket;
                    fds[i + 1].events = POLLIN;
                    break;
                }
            }
            if (i != MAX_CLIENTS)
            {
                if (newClientConnected(clients[i]) != 1)
                    numClients++;
            }
            else
                close(clientSocket);
        } 
        else
            close(clientSocket);
    }
}

void Server::run() {
    // Aggiungi il socket del server all'array di pollfd
    struct pollfd fds[MAX_CLIENTS + 1];
    memset(fds, 0, sizeof(fds));
    fds[0].fd = skt.getSocket();
    fds[0].events = POLLIN;
    int numClients = 0; // Numero attuale di client connessi
    while (isServerRunning) 
    {
        int ret = poll(fds, numClients + 1, 1000);
        if (ret < 0) {
            perror("Poll error");
            break;
        }
        else if (ret == 0)
            continue;
        newClientHandler(fds, numClients);
        for (int i = 0; i < numClients; ++i)
            if (fds[i + 1].revents & POLLIN)
                messageHandler(clients[i]);
    }
    for (int i = 0; i < numClients; ++i)
        if (clients[i].getSocket() > 0)
            close(clients[i].getSocket());
}
