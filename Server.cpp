/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/11 17:27:53 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/19 19:07:46 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(const int& p, const std::string& pass) : serverName(":ludri"), serverPassword("ktm"), userPassword(pass), port(p) , isServerRunning(true){
	if (port != SERVER_PORT)
		throw std::runtime_error("Error: Wrong port!");
	if(userPassword != serverPassword)
		throw std::runtime_error("Error: Wrong password!");
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        User cl;
        cl.setSocket(-1);
        clients.push_back(cl);
    }
    getMyIP();
	socketInit();
	binding();
}

Server::~Server(){
	close(serverSocket);
}

/**
 * @brief Socket init with non-blocking I/O
 * @throws Failed 
 * @throws Non-block
 * @throws Non reusable
 */
void Server::socketInit(){
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) 
		throw std::runtime_error("Socket failed");
	int flags = fcntl(serverSocket, F_GETFL, 0);
    if (fcntl(serverSocket, F_SETFL, flags | O_NONBLOCK) < 0)
		throw std::runtime_error("Socket non-blocking error");
	int reuseAddr = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr)) < 0)
		throw std::runtime_error("Socket can't be reused");
	memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(IP);
	std::cout << "SERVER IP: " << IP << std::endl;
    //controlla puliza serverAddr
}

void Server::binding(){
    if (bind(serverSocket, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) < 0)
      throw std::runtime_error("Binding error");
    if (listen(serverSocket, SOMAXCONN) < 0)
       throw std::runtime_error("Listening error");
}

void Server::getMyIP(){
	struct hostent *host_entry;
	int	host;

	host = gethostname(hostname, sizeof(hostname));
	if (host < 0)
		throw std::runtime_error("Hostname error");
	host_entry = gethostbyname(hostname);
	if(!host_entry)
		throw std::runtime_error("Host entry error");
    IP = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
}

void Server::newClientConnected(User& user)
{
	user.setIP(IP);
	for (int i = 0; i < 5; ++i)
    {
		char buffer[1024];
		ssize_t bytesRead = recv(user.getSocket(), buffer, sizeof(buffer) - 1, 0);
		buffer[bytesRead] = '\0';
		if (!strncmp(buffer, "NICK", 4))
        {
            std::vector <User> ::iterator it = clients.begin();
            for (; it != clients.end(); ++it) {   
                if (it->getNick() == trimMessage(buffer, 5)) {
                    std::string ERR_NICKNAMEINUSE = serverName + " 433 * " + it->getNick() + " :Nickname is already in use\r\n";
                    send(user.getSocket(), ERR_NICKNAMEINUSE.c_str(),ERR_NICKNAMEINUSE.length(), 0);
                    return ;
                }
            }
			user.setNick(&((trimMessage(buffer, 5))[0]));
			std::memset(buffer, 0, sizeof(buffer));
		}
        else if (!strncmp(buffer, "USER", 4)) {
            user.setUser(std::strtok(&buffer[5], " "));
            break;
        }
	}
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

void Server::newClientHandler(struct pollfd* fds, int& numClients) {
    if (fds[0].revents & POLLIN)
    {
        int clientSocket = accept(serverSocket, NULL, NULL);
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
            if (i != MAX_CLIENTS) {
                numClients++;
                newClientConnected(clients[i]);
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
    fds[0].fd = serverSocket;
    fds[0].events = POLLIN;
    int numClients = 0; // Numero attuale di client connessi
    while (1) 
    {
        int ret = poll(fds, numClients + 1, 1000);
        if (ret < 0) {
            if (errno == EINTR) {
                perror("Poll error");
                break;
            }
        }
        else if (ret == 0)
            continue;
        newClientHandler(fds, numClients);
        for (int i = 0; i < numClients; ++i)
            if (fds[i + 1].revents & POLLIN)
                messageHandler(clients[i]);
        if (!isServerRunning)
            break;
    }
    for (int i = 0; i < numClients; ++i)
        if (clients[i].getSocket() > 0)
            close(clients[i].getSocket());
}
