/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hznagui <hznagui@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 11:08:29 by hznagui           #+#    #+#             */
/*   Updated: 2024/02/20 11:08:29 by hznagui          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"
#define PORT 6697
#define PASSWORD "password"

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <port> <password>\n";
        return -1;
    }
    try {
        server ircServer(std::stoi(argv[1]), argv[2]);
        ircServer.run();
    } catch (server::serverException &e) {
        std::cerr << "Error in main: " << e.what() << "\n";
        return -1;
    }
    return 0;
}