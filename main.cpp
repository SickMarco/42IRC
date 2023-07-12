/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/11 17:23:52 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/12 19:02:08 by mbozzi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Irc.hpp"

int main(int ac, char **av)
{
	if (ac == 3)
	{
		const int port = atoi(av[1]);
		const std::string pswd = av[2];
		try {
			Server server(port, pswd);
		}
		catch(const std::exception& e) {
			std::cerr << e.what() << '\n';
			return 1;
		}

		Server server(port, pswd);
		server.tester();
	}
	else
		std::cerr << "Error: insert <port> <password>" << std::endl;
	return 0;
}