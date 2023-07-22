/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channels.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/22 10:58:14 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/22 11:31:09 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include "User.hpp"

struct Channel {
	std::string topic;
	std::vector<User> clients;
	std::vector<std::string> operators;
};

class Channels
{
private:
	std::string serverName;
	std::string hostname;
	std::map<std::string, Channel > channels;
	bool channelExist(User& user, const std::string& channelName);
	void createNewChannel(const User& user, const std::string& channelName, bool& setOp);
	void channelOperators(const User& user, const std::string& channelName, bool& setOp);
	void joinMessageSequence(const User& user, const std::string& channelName);
	void multiChannelJoin(User& user, std::string channelName);

public:
	Channels();
	Channels(const std::string& serverName, const std::string& hostname);	
	~Channels();

	std::map<std::string, Channel >& getChannels();

	void joinChannel(std::string channelName, User &client);
	void leaveChannel(std::string channelName, User &client, std::string message);
	int	messageToChannel(User& user, std::string buffer);
};




#endif