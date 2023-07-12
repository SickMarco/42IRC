/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/11 17:27:53 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/12 19:00:27 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(const int& p, const std::string& pass) : port(p), serverPassword("ShFg]PWWaQY") , userPassword(pass){
	encryptPassword();
	if (port != SERVER_PORT)
		throw std::runtime_error("Error: Wrong port!");
	if(userPassword != serverPassword)
		throw std::runtime_error("Error: Wrong password!");
	start();
}

Server::~Server(){
	close(serverSocket);
}

void Server::encryptPassword(){
	std::string key = "ludro";
    for (std::size_t i = 0; i < userPassword.length(); ++i) {
       userPassword[i] = ((userPassword[i] - 32) + (key[i % key.length()] - 32)) % 95 + 32;
    }
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
    serverAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
}

void Server::binding(){
    if (bind(serverSocket, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) < 0)
      throw std::runtime_error("Binding error");
    if (listen(serverSocket, SOMAXCONN) < 0)
       throw std::runtime_error("Listening error");
}

void Server::tester(){
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
		} else if (ret == 0) {
			continue;
		} else {
			// Ci sono connessioni in arrivo, esegui accept
			if (fds[0].revents & POLLIN) {
				clientAddrLen = sizeof(clientAddr);
				clientSocket = accept(serverSocket, (struct sockaddr*) &clientAddr, &clientAddrLen);
				if (clientSocket < 0) {
					if (errno != EWOULDBLOCK && errno != EAGAIN) {
						perror("Accept error");
						break;
					}
				} else {
					std::string welcomeMsg = "Benvenuto nel server IRC!\n";
					if (send(clientSocket, welcomeMsg.c_str(), strlen(welcomeMsg.c_str()), 0) < 0) {
						perror("Send error");
						close(clientSocket);
						break;
					}
					// Ricevi un messaggio dal client
					while(1)
					{
						char buffer[1024];
						ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
						if (bytesRead < 0) {
							perror("Receive error");
							close(clientSocket);
							break ;
						} else if (bytesRead == 0) {
							// Il client ha chiuso la connessione
							close(clientSocket);
							break ;
						}
						buffer[bytesRead] = '\0';
						if (!strcmp(buffer, "exit\n"))
							break ;
						printf("Messaggio ricevuto dal client: %s", buffer);
					}
					close(clientSocket);
					break; 
				}
			}
		}
	}
}

void Server::start(){
	socketInit();
	binding();
}