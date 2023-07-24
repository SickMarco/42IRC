/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channels.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/22 10:58:14 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/24 15:38:50 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include <set>
#include "User.hpp"

struct Channel {
	std::string topic;
	std::vector<User> clients;
	std::vector<User> operators;
	bool topicMode;
	bool inviteOnly;
	std::set <std::string> banlist;
	std::set <std::string> invitelist;
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
	bool checkOperator(const User& user, const std::string& channelName);
	void joinMessageSequence(const User& user, const std::string& channelName);
	void multiChannelJoin(User& user, std::string channelName);
	void setTopic(const User& user, const std::string& channelName, const std::string& arg);
	void sendToAll(const std::string& channelName, const std::string& message);

public:
	Channels();
	~Channels();

	void init(const std::string& serverName, const std::string& hostname);	
	std::map<std::string, Channel>& getChannels();

	void joinChannel(User& user, std::string channelName);
	void leaveChannel(User& user, std::string channelName, std::string message);
	int	messageToChannel(const User& user, std::string buffer);
	void topic(const User& user, std::string buffer);
	void setModeTopic(const User& user, const std::string& channelName, const std::string& flag);
	void setModeOperator(const User& user, std::string buffer, const std::string& flag);
	void setModeInviteOnly(const User& user, const std::string& channelName, const std::string& flag);
};




#endif