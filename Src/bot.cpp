/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/27 18:22:46 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/27 19:14:46 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channels.hpp"
#include <ctime>

void Channels::botCommand(const User& user, const std::string& channelName, std::string buffer){
	std::string cmd = buffer.substr(buffer.find("!bot ") + 5);
	if (cmd.find("help") != cmd.npos){
		std::string botHelp = ":Mimmomodem PRIVMSG #" + channelName + " :List of bot commands:\r\nmarasco\r\nlello\r\nrickroll\r\nbobbe\r\ncoin\r\n";
		send(user.getSocket(), botHelp.c_str(), botHelp.length(), 0);
	}
	else if (cmd.find("marasco") != cmd.npos)
		std::system("xdg-open https://youtu.be/_tGtYitmLLM?t=108");
	else if (cmd.find("lello") != cmd.npos)
		std::system("https://youtu.be/B8aUQyMQICo?t=318");
	else if (cmd.find("rickroll") != cmd.npos)
		std::system("xdg-open https://www.youtube.com/watch?v=dQw4w9WgXcQ");
	else if (cmd.find("bobbe") != cmd.npos)
		std::system("xdg-open https://www.youtube.com/watch?v=aRTA4rJjeig");
	else if (cmd.find("coin") != cmd.npos) {
		std::string coin = ":Mimmomodem PRIVMSG #" + channelName + " :";
		std::srand(std::time(NULL));
		int res = std::rand() % 2;
		if (res == 0)
			coin += "HEADS\r\n";
		else 
			coin += "TAILS\r\n";
		send(user.getSocket(), coin.c_str(), coin.length(), 0);
	}
	else if (cmd.find("ff on") != cmd.npos && findClientByName(channels[channelName].operators, user.getNick()) != -1)
	{
		channels[channelName].censorship = true;
		sendToAll(channelName, "Cnsorship is ON\r\n");
	}
	else if (cmd.find("ff off") != cmd.npos && findClientByName(channels[channelName].operators, user.getNick()) != -1)
	{
		channels[channelName].censorship = false;
		sendToAll(channelName, "Cnsorship is OFF\r\n");
	}
}