/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channels.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/22 10:58:14 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/25 18:37:28 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include <set>
#include <sstream>
#include "User.hpp"

struct Channel {
	std::string topic;
	std::vector<User> clients;
	std::vector<User> operators;
	bool topicMode;
	bool inviteOnly;
	bool userLimit;
	unsigned int userMax;
	std::set <std::string> banlist;
	std::set <std::string> invitelist;
	std::string passKey;
};

class Channels
{
private:
	std::string serverName;
	std::string hostname;
	std::map<std::string, Channel > channels;

	void addUserToChannel(User& user, std::string channelName);
	void createNewChannel(const User& user, const std::string& channelName, bool& setOp);
	void channelOperators(const User& user, const std::string& channelName, bool& setOp);
	int  checkChannelModes(const User& user, const std::string channelName, const std::string& key);
	bool checkOperator(const User& user, const std::string& channelName);
	void joinMessageSequence(const User& user, const std::string& channelName);
	void setTopic(const User& user, const std::string& channelName, const std::string& arg);

public:
	Channels();
	~Channels();

	void init(const std::string& serverName, const std::string& hostname);	
	std::map<std::string, Channel>& getChannels();

	int joinChannel(User& user, std::string channelName, std::string key);
	void leaveChannel(User& user, std::string channelName, std::string message);
	int	messageToChannel(const User& user, std::string buffer);
	void topic(const User& user, std::string buffer);
	void setModeTopic(const User& user, const std::string& channelName, const std::string& flag);
	void setModeOperator(const User& user, std::string buffer, const std::string& flag);
	void sendToAll(const std::string& channelName, const std::string& message);
	void setModeInviteOnly(const User& user, const std::string& channelName, const std::string& flag);
	void setModeKey(const User& user, std::string buffer, std::string mode);
	bool channelExist(std::string channelName);
	void setModeUserLimit(const User& user, std::string buffer, const std::string& flag);
	void multiChannelJoin(User& user, std::string buffer);
	std::vector<std::string> split(std::string s, char delimiter);
};

#endif