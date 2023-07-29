/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/27 18:22:46 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/28 18:39:49 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channels.hpp"
#include <ctime>

void Channels::botCommand(const User& user, const std::string& channelName, std::string buffer){
	std::string cmd = "";
	if (buffer.find("!bot ") == buffer.npos)
		cmd = buffer.substr(buffer.find("!bot ") + 5);
	if (cmd.find("help") != cmd.npos){
		std::string botHelp = ":Mimmomodem PRIVMSG #" + channelName + " :List of bot commands:\r\nmarasco\r\nlello\r\nrickroll\r\nbobbe\r\ncoin\r\nff on\\off\r\n";
		send(user.getSocket(), botHelp.c_str(), botHelp.length(), 0);
	}
	else if (cmd.find("marasco") != cmd.npos) {
		std::string marasco = ":Mimmomodem PRIVMSG #" + channelName + " :https://youtu.be/_tGtYitmLLM?t=108\r\n";
		sendToAll(channelName, marasco);
	}
	else if (cmd.find("lello") != cmd.npos) {
		std::string lello = ":Mimmomodem PRIVMSG #" + channelName + " :https://youtu.be/B8aUQyMQICo?t=320\r\n";
		sendToAll(channelName, lello);
	}
	else if (cmd.find("rickroll") != cmd.npos){
		std::string rickroll = ":Mimmomodem PRIVMSG #" + channelName + " :https://www.youtube.com/watch?v=dQw4w9WgXcQ\r\n";
		sendToAll(channelName, rickroll);
	}
	else if (cmd.find("bobbe") != cmd.npos){
		std::string bobbe = ":Mimmomodem PRIVMSG #" + channelName + " :https://www.youtube.com/watch?v=aRTA4rJjeig\r\n";
		sendToAll(channelName, bobbe);
	}
	else if (cmd.find("coin") != cmd.npos) {
		std::string coin = ":Mimmomodem PRIVMSG #" + channelName + " :";
		std::srand(std::time(NULL));
		int res = std::rand() % 2;
		if (res == 0)
			coin += "HEADS\r\n";
		else 
			coin += "TAILS\r\n";
		sendToAll(channelName, coin);
	}
	else if (cmd.find("ff on") != cmd.npos && findClientByName(channels[channelName].operators, user.getNick()) != -1)
	{
		channels[channelName].censorship = true;
		sendToAll(channelName, "Censorship ON\r\n");
	}
	else if (cmd.find("ff off") != cmd.npos && findClientByName(channels[channelName].operators, user.getNick()) != -1)
	{
		channels[channelName].censorship = false;
		sendToAll(channelName, "Censorship OFF\r\n");
	}
	else {
		std::string notFound = ":Mimmomodem PRIVMSG #" + channelName + " :Command not found. [!bot help for command list]\r\n";
		send(user.getSocket(), notFound.c_str(), notFound.length(), 0);
	}
}