/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/22 11:44:39 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/22 11:57:01 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <iostream>
#include <netinet/in.h>
#include <fcntl.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>


#define SERVER_PORT 6667

class Socket
{
private:
	int serverSocket;
	struct sockaddr_in serverAddr;
	
public:
	Socket();
	~Socket();

	void getMyIP(std::string& hostnm, std::string& IP);
	void socketInit(std::string IP);
	void binding();

	int getSocket() const;
};




#endif