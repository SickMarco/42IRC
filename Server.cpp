/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/11 17:27:53 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/13 20:46:34 by mbozzi           ###   ########.fr       */
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
    serverAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
}

void Server::binding(){
    if (bind(serverSocket, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) < 0)
      throw std::runtime_error("Binding error");
    if (listen(serverSocket, SOMAXCONN) < 0)
       throw std::runtime_error("Listening error");
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
		} else if (ret == 0) {
			continue;
		} else {
			// Ci sono connessioni in arrivo, esegui accept
			if (fds[0].revents & POLLIN) {
				user.socketAccept(serverSocket);
				if (user.getSocket() < 0) {
					if (errno != EWOULDBLOCK && errno != EAGAIN) {
						perror("Accept error");
						break;
					}
				} else {
					user.setNick();
					std::string welcomeMsg = "Benvenuto nel server IRC!\n";
					if (send(user.getSocket(), welcomeMsg.c_str(), strlen(welcomeMsg.c_str()), 0) < 0) {
						perror("Send error");
						close(user.getSocket());
						break;
					}
					// Ricevi un messaggio dal client
					while(1)
					{
						char buffer[1024];
						ssize_t bytesRead = recv(user.getSocket(), buffer, sizeof(buffer) - 1, 0);
						if (bytesRead < 0) {
							perror("Receive error");
							close(user.getSocket());
							break ;
						} else if (bytesRead == 0) {
							// Il client ha chiuso la connessione
							close(user.getSocket());
							break ;
						}
						buffer[bytesRead] = '\0';
						std::string ktm = buffer;
						if (!strncmp(buffer, "JOIN", 4))
						{
							if (send(user.getSocket(), ":mbozzi JOIN #ktm\r\n", 20, 0) < 0)
								perror("Send error");
						}
						else if (!strncmp(buffer, "NICK", 4))
						{
							user.changeNickname(ktm.substr(5, ktm.find('\n') - 1));
						}
						printf("Messaggio ricevuto dal client: %s", buffer);
					}
					close(user.getSocket());
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