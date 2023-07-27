/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/27 18:22:46 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/27 18:55:18 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channels.hpp"

void Channels::botCommand(const User& user, const std::string& channelName, std::string buffer){
	std::string cmd = buffer.substr(buffer.find("!bot ") + 5);
	if (cmd.find("help") != cmd.npos){
		std::string botHelp = ":" + user.getNick() + " PRIVMSG #" + channelName + " :\r\nBot [List of bot commands]\r\nmarasco\r\nlello\r\nrickroll\r\n";
		send(user.getSocket(), botHelp.c_str(), botHelp.length(), 0);
	}
	else if (cmd.find("marasco") != cmd.npos)
		std::system("xdg-open https://www.youtube.com/watch?v=_tGtYitmLLM&t=2s");
	else if (cmd.find("lello") != cmd.npos)
		std::system("xdg-open https://www.youtube.com/watch?v=_tGtYitmLLM&t=2s");
	else if (cmd.find("rickroll") != cmd.npos)
		std::system("xdg-open https://www.youtube.com/watch?v=dQw4w9WgXcQ");
}