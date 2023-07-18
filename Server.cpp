/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/11 17:27:53 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/18 19:18:56 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(const int& p, const std::string& pass) : serverName(":ludri"), serverPassword("ktm"), userPassword(pass), port(p){
	if (port != SERVER_PORT)
		throw std::runtime_error("Error: Wrong port!");
	if(userPassword != serverPassword)
		throw std::runtime_error("Error: Wrong password!");
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
	std::cout << IP << std::endl;
	std::cout << hostname << std::endl;
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

void Server::newClientConnected(User& user){
	user.setIP(IP);
	for (int i = 0; i < 5; ++i) {
		char buffer[1024];
		ssize_t bytesRead = recv(user.getSocket(), buffer, sizeof(buffer) - 1, 0);
		buffer[bytesRead] = '\0';
		if (!strncmp(buffer, "NICK", 4)) {
            for (size_t i = 0; i < MAX_CLIENTS; ++i) {
                if (clients[i].getNick() == trimMessage(buffer, 5)) {
                    std::string nameUsed = serverName + " 433 * " + clients[i].getNick() + " :Nickname is already in use\r\n";
                    send(user.getSocket(), nameUsed.c_str(),nameUsed.length(), 0);
                    return ;
                }
            }
			user.setNick(buffer);
			std::memset(buffer, 0, sizeof(buffer));
		}
        else if (!strncmp(buffer, "USER", 4)) {
            user.setUser(std::strtok(&buffer[5], " "));
            break;
        }
	}
    std::string RPL_WELCOME = serverName + " 001 " + user.getNick() + " :Welcome to the Internet Relay Network " + user.getNick() + "!" + user.getUser() + "@" + hostname + "\r\n";
    printStringNoP(RPL_WELCOME.c_str(), RPL_WELCOME.length());
    send(user.getSocket(), RPL_WELCOME.c_str(), strlen(RPL_WELCOME.c_str()), 0);
}

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
            std::cout << "BUFFER: " << buffer << std::endl;
            std::string buf = removeCRLF(&(buffer[5]));
            
            std::string token = std::strtok(&(buf[0]), ","); 
            while (!token.empty())
            {
                leaveChannel(&(token[1]), user);
              std::string token = std::strtok(&(buf[0]), ","); 
            }
        }
		std::memset(buffer, 0, sizeof(buffer));
}

int Server::messageToPrivate(User& user, std::string buffer)
{
    std::string name = buffer.substr(0, buffer.find(' '));
    std::string mex = buffer.substr(name.length() + 1, std::string::npos);

    int clientSocket = -1;
    size_t i;
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].getNick() == name) {
            clientSocket = clients[i].getSocket();
            break;
        }
    }
    std::string privmsg = ":" + user.getNick() + " PRIVMSG " + name + " " + mex.substr(0, mex.length()) + "\r\n";
    send(clientSocket,  privmsg.c_str(),  privmsg.length(), 0);
    return 0;
}

void Server::tester()
{
    for (int i = 0; i < MAX_CLIENTS; ++i)
        clients[i].setSocket(-1);
    // Aggiungi il socket del server all'array di pollfd
    struct pollfd fds[MAX_CLIENTS + 1];
    fds[0].fd = serverSocket;
    fds[0].events = POLLIN;
    int numClients = 0; // Numero attuale di client connessi
    while (1) {
        int ret = poll(fds, numClients + 1, 1000);
        if (ret < 0) {
            if (errno == EINTR) {
                perror("Poll error");
                break;
            }
        } else if (ret == 0) {
            continue;
        }
        // Controlla se ci sono nuove connessioni
        if (fds[0].revents & POLLIN) {
            int clientSocket = accept(serverSocket, NULL, NULL);
            if (clientSocket < 0) {
                perror("Accept error");
                continue;
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
                if (i == MAX_CLIENTS) {
                    close(clientSocket);
                } else {
                    // Incrementa il numero di client connessi
                    numClients++;
                    newClientConnected(clients[i]);
                }
            } else {
                close(clientSocket);
            }
        }
        // Controlla se ci sono dati in arrivo dai client esistenti
        for (int i = 0; i < numClients; ++i) {
            if (fds[i + 1].revents & POLLIN) {
                messageHandler(clients[i]);
            }
        }
    }
    // Chiudi tutte le connessioni client
    for (int i = 0; i < numClients; ++i) {
        close(clients[i].getSocket());
    }
}