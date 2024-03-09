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
    try {
        bot bot(argv[1], std::stoi(argv[2]), "bot", "bot", "bot",argv[3]);
        bot.connectToServer();
        bot.listenToServerAndRespond();
    } catch (bot::botException &e) {
        std::cerr << "Error in bot: " << e.what() << "\n";
        return -1;
    }
}
