/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/11 17:27:53 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/14 18:46:30 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(const int& p, const std::string& pass) : port(p), serverPassword("ktm") , userPassword(pass){
	if (port != SERVER_PORT)
		throw std::runtime_error("Error: Wrong port!");
	if(userPassword != serverPassword)
		throw std::runtime_error("Error: Wrong password!");
	start();
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
	std::string welcomeMsg = "Benvenuto nel server IRC!\r\n";
	if (send(user.getSocket(), welcomeMsg.c_str(), strlen(welcomeMsg.c_str()), 0) < 0) {
		perror("Send error");
		close(user.getSocket());
	}
	char buffer[1024];
	ssize_t bytesRead = recv(user.getSocket(), buffer, sizeof(buffer) - 1, 0);
	buffer[bytesRead] = '\0';
	user.setNick(buffer);
	printStringNoP(buffer, static_cast<std::size_t>(bytesRead));
	std::cout << " " << bytesRead << std::endl;
	std::memset(buffer, 0, sizeof(buffer));
}

void Server::messageHandler(User& user){
	while(1)
	{
		char buffer[1024];
		ssize_t bytesRead = recv(user.getSocket(), buffer, sizeof(buffer) - 1, 0);
		if (bytesRead < 0) {
			perror("Receive error");
			close(user.getSocket());
			break ;
		} else if (bytesRead == 0) {
			close(user.getSocket());
			break ;
		}
		buffer[bytesRead] = '\0';
		printStringNoP(buffer, static_cast<std::size_t>(bytesRead));
		/* if(!strncmp(buffer, "NICK", 4))
			user.changeNickname(buffer); */
		//printStringNoP(buffer, static_cast<std::size_t>(bytesRead));
		//std::cout << bytesRead << std::endl;
		std::memset(buffer, 0, sizeof(buffer));
	}
}

void Server::tester(){
	User user;
	while (1)
	{
		struct pollfd fds[1];
		fds[0].fd = serverSocket;
		fds[0].events = POLLIN;

		int ret = poll(fds, 1, 1000);
		if (ret < 0) {
			if (errno == EINTR) {
				perror("Poll error");
				break;
			}
		} 
		else if (ret == 0)
			continue;
		if (fds[0].revents & POLLIN) {
			user.socketAccept(serverSocket);
			newClientConnected(user);
			messageHandler(user);
		}
	}
}

void Server::start(){
	getMyIP();
	socketInit();
	binding();
}

void Server::printStringNoP(const char* str, std::size_t length) {
    for (std::size_t i = 0; i < length; ++i) {
        if (std::isprint(static_cast<unsigned char>(str[i]))) {
            std::cout << str[i]  << std::flush;
        } else {
            std::cout << "\\x" << std::hex << static_cast<int>(str[i])  << std::flush;
            std::cout << std::dec;
        }
    }
	std::cout << std::endl << std::flush;
}