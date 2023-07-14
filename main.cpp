/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbozzi <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/11 17:23:52 by mbozzi            #+#    #+#             */
/*   Updated: 2023/07/14 15:03:43 by mbozzi           ###   ########.fr       */
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
			server.tester();
		}
		catch(const std::exception& e) {
			std::cerr << e.what() << '\n';
			return 1;
		}
	}
	else
		std::cerr << "Error: insert <port> <password>" << std::endl;
	return 0;
}