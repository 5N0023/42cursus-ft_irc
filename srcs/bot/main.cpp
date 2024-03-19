/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hznagui <hznagui@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 11:08:59 by hznagui           #+#    #+#             */
/*   Updated: 2024/02/20 11:08:59 by hznagui          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "bot.hpp"
int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " <server> <port> <password>\n";
        return -1;
    }
    try
    {
        std::cerr << "Creating bot\n";
        bot bot(argv[1], atoi(argv[2]), "bot", "bot", "bot", argv[3]);
        std::cerr << "Connecting to server\n";
        bot.connectToServer();
        std::cerr << "Listening to server and responding\n";
        bot.listenToServerAndRespond();
    }
    catch (std::exception &e)
    {
        std::cerr << "Error in bot: " << e.what() << "\n";
        return -1;
    }
}
