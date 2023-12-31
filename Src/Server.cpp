/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/11 17:27:53 by mbozzi            #+#    #+#             */
/*   Updated: 2023/08/01 14:58:50 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(const int& p, const std::string& pass) : serverName(":ludri"), serverPassword("ktm"),  port(p), userPassword(pass), isServerRunning(true), epollFd(-1){
	if (port != SERVER_PORT)
		throw std::runtime_error("Error: Wrong port!");
	if(userPassword != serverPassword)
		throw std::runtime_error("Error: Wrong password!");
    clients.resize(MAX_CLIENTS);
    skt.getMyIP(hostname, IP);
	skt.socketInit(IP);
	skt.binding();
    channels.init(serverName, hostname);
	clientsConnected = 0;
    std::cout << "Server IP: \033[0;35m" << IP << "\033[0;37m" << std::endl;
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
    send(user.getSocket(), RPL_WELCOME.c_str(), RPL_WELCOME.length(), sndFlags);
    send(user.getSocket(), RPL_YOURHOST.c_str(), RPL_YOURHOST.length(), sndFlags);
    send(user.getSocket(), RPL_CREATED.c_str(), RPL_CREATED.length(), sndFlags);
    send(user.getSocket(), RPL_MOTD.c_str(), RPL_MOTD.length(), sndFlags);
    send(user.getSocket(), RPL_ENDOFMOTD.c_str(), RPL_ENDOFMOTD.length(), sndFlags);
}

bool Server::checkPassword(User& user, const std::string& PASS){
    if (PASS != serverPassword && !serverPassword.empty())
    {
        std::string ERR_PASSWDMISMATCH = serverName + " ERR_PASSWDMISMATCH :Password incorrect.\r\n";
        send(user.getSocket(), ERR_PASSWDMISMATCH.c_str(), ERR_PASSWDMISMATCH.length(), sndFlags);
        std::string quitmsg = "ERROR :Closing Link: " + IP + " (Connection refused by server)\r\n";
        send(user.getSocket(), quitmsg.c_str(), quitmsg.length(), sndFlags);
        
        close(user.getSocket());
        user.setSocket(-1);
        user.reset();
        clientsConnected--;
        return false;
    }
    return true;
}

int Server::findClientIndex(int fd) {
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (fd == clients[i].getSocket())
            return i;
    }
    return -1;
}

int Server::addSocketToEpoll(int newSocket)
{
    struct epoll_event event;
    memset(&event, 0 , sizeof(event));
    event.events = EPOLLIN | EPOLLET; //set reading data with non blocking
    event.data.fd = newSocket;
    // Add socket to epoll and monitor events
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, newSocket, &event) == -1) {
        perror("Error adding socket to epoll");
        close(epollFd);
        return -1;
    }
    return 0;
}

int Server::newClientHandler() {
    int i = findClientIndex(-1); 
    if (i < 0)
        return -1;
    // Accept client connection and create a new socket
    int clientSocket = accept(skt.getSocket(), (struct sockaddr*)&clients[i].getAddr(), &clients[i].getAddrLen());
    // Set the client socket to non-blocking mode
    if (clientSocket < 0 || fcntl(clientSocket, F_SETFL, O_NONBLOCK) < 0) {
        clientSocket < 0 ? perror("Accept error") : perror("Client socket non-blocking error");
        return -1;
    }
    // Set user socket
    clients[i].setSocket(clientSocket);
    clients[i].setIP(IP);
    clientsConnected++;
    // Add the new socket to epollun 
    if (addSocketToEpoll(clientSocket) < 0){
        close(clientSocket);
        clients[i].setSocket(-1);
        clientsConnected--;
        return -1;
    }
    return i;
}

void Server::run() {

    // Create epoll istance
    epollFd = epoll_create1(0);
    if (epollFd < 0) {
        perror("epoll");
        return ;
    }
    // Add server socket to epoll
    if (addSocketToEpoll(skt.getSocket()) < 0)
        return ;
    
    struct epoll_event events[MAX_CLIENTS + 1];
    memset(events, 0 , sizeof(events));

    int triggered;
    while (isServerRunning) {
        // Waiting for events
        triggered = epoll_wait(epollFd, events, MAX_CLIENTS, 0);
        if (triggered < 0) {
            perror("epoll_wait error");
            break;
        }
        // For each triggered event
        for (int i = 0; i < triggered; i++) {
            int clientSocket = events[i].data.fd;
            // New client connection
            if (clientSocket == skt.getSocket()) {
                    if (clientsConnected == MAX_CLIENTS || newClientHandler() < 0)
                        continue;
            }
            else if (clientSocket != -1)
                messageHandler(clients[findClientIndex(clientSocket)]);
        }
    }
    // Close all connections
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].getSocket() != -1)
            close(clients[i].getSocket());
    }
    close(epollFd);
}