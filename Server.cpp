/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/11 17:27:53 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/16 20:18:09 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(const int& p, const std::string& pass) : port(p), serverPassword("ktm") , userPassword(pass){
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

std::string removeCRLF(const char* buffer){
    std::string input = buffer;
	std::string ret = input.substr(0, input.length() - 1);
    return ret;
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
	for (int i = 0; i < 4; ++i){
		char buffer[1024];
		ssize_t bytesRead = recv(user.getSocket(), buffer, sizeof(buffer) - 1, 0);
		buffer[bytesRead] = '\0';
		if (!strncmp(buffer, "NICK", 4))
		{
			user.setNick(buffer);
			std::memset(buffer, 0, sizeof(buffer));
		}
	}
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
        else if (!strncmp(buffer, "PRIVMSG ", 8))
        {
            if (buffer[8] == '#')
                messageToChannel(user, removeCRLF(&(buffer[9])));
            else
                messageToPrivate(user, removeCRLF(&(buffer[8])));
        }
		std::memset(buffer, 0, sizeof(buffer));
}

int Server::messageToChannel(User& user, std::string buffer)
{
    std::string channelName = buffer.substr(0, buffer.find(' '));
    std::string mex = buffer.substr(channelName.length() + 1, std::string::npos);

    // Find the channel
    std::map<std::string, std::vector<User> >::iterator it = channels.find(channelName);
    std::cout << "SIZE " << channels.size() << std::endl;
	for (std::map<std::string, std::vector<User> >::iterator start = channels.begin(); start != channels.end(); ++start)
		printStringNoP(start->first.c_str(), start->first.length());
    
    if (it != channels.end())
    {
        // Channel exists, send the message to all clients in the channel
        std::vector<User> channelClients = it->second;
        std::string privmsg = ":" + user.getNick() + " PRIVMSG #" + channelName + " " + mex.substr(0, mex.length()) + "\n";
        
        printStringNoP(privmsg.c_str(), privmsg.length());
        for (size_t i = 0; i < channelClients.size(); ++i)
        {
            if (channelClients[i].getSocket() != -1)
            {
                std::cout << "CLIENT FOUND" << std::endl;
                send(channelClients[i].getSocket(), privmsg.c_str(), privmsg.length(), 0);
            }
        }
    }
    else
        std::cout << "CHANNEL NOT FOUND" << std::endl;
    return 0;
}

int Server::messageToPrivate(User& user, std::string buffer)
{
    std::string name = buffer.substr(0, buffer.find(' '));
    std::string mex = buffer.substr(name.length() + 1, std::string::npos);

    int clientSocket = -1;
    size_t i;
    for (i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].getNick() == name)
        {
            clientSocket = clients[i].getSocket();
            break;
        }
    }
    //std::string privmsg = ":" + user.getNick() + "!" + hostname + "@" + IP + " PRIVMSG " + name + " " + mex.substr(0, mex.length() - 1) + "\r\n";
    std::string privmsg = ":" + user.getNick() + " PRIVMSG " + name + " " + mex.substr(0, mex.length() - 1) + "\r\n";
    send(clientSocket,  privmsg.c_str(),  privmsg.length(), 0);
    return 0;
}

void Server::tester()
{
    // Inizializza l'array di client
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        clients[i].setSocket(-1); // Resetta lo stato di ogni client
    }

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


void Server::joinChannel(std::string channelName, User client)
{
    // Check if the channel exists
    std::map<std::string, std::vector<User> >::iterator it = channels.find(channelName);
    
	if (it != channels.end())
    {
        it->second.push_back(client); // Channel exists, add the client to the channel participants
        std::cout << "JOINED " << it->first << std::endl;
    }
	else
	{
        // Channel doesn't exist, create a new channel and add the client
        std::vector<User> newChannel;
        newChannel.push_back(client);
        channels[channelName] = newChannel;
    }
    std::string join = ":" + client.getNick() + " JOIN :" + channelName + "\n";
	printStringNoP(channelName.c_str(), channelName.length());
	send(client.getSocket(), join.c_str(), join.length(), 0);
}