/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hznagui <hznagui@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 11:08:55 by hznagui           #+#    #+#             */
/*   Updated: 2024/02/20 11:08:55 by hznagui          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "bot.hpp"

bot::bot(std::string server, int serverPort, std::string nick, std::string user, std::string realname,std::string serverPassword)
{
    this->server = server;
    this->serverPassword = serverPassword;
    this->serverPort = serverPort;
    this->nick = nick;
    this->user = user;
    this->realname = realname;
    socket = ::socket(AF_INET, SOCK_STREAM, 0);

    matches = getMatches();

}

void bot::connectToServer()
{
    struct sockaddr_in server_address;
    if (socket < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(CONNECTION_PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0)
    {
        perror("Invalid address / Address not supported");
        exit(EXIT_FAILURE);
    }

    if (connect(socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0)
    {
       throw botException("Connection failed");
    }
    // std::string passwordSend ="PASS "+ serverPassword + " \r\n",
    send(socket, "PASS password\r\n",strlen("PASS password\r\n"),0);
    usleep(500);
    send(socket, "NICK bot\r\n", strlen("NICK bot\r\n"), 0);
    usleep(500);
    send(socket, "USER bot 0 * bot\r\n", strlen("USER bot 0 * bot\r\n"), 0);
    usleep(500);
    // receive welcome message
    char receive_buffer[1024];
    int valread = recv(socket, receive_buffer, 1024, 0);
    std::string message = receive_buffer;
    if (valread > 0)
    {
        if (message.find("Welcome") == std::string::npos)
            throw botException("Verify if  PASSWORD is correct");
    }
    else if (valread == 0)
        throw botException("Server disconnected");
    else
        throw botException("Error in recv");
}


void bot::listenToServerAndRespond()
{
    char receive_buffer[1024];
    int valread;
    while (1)
    {
        valread = recv(socket, receive_buffer, 1024, 0);
        std::string message = receive_buffer;
        if (valread > 0)
        {
            if (message.find("PRIVMSG") != std::string::npos)
            {
                std::string sender = message.substr(1, message.find("!") - 1);
                usleep(500);
                if (matches.size() == 0)
                {
                     std::string reply = "PRIVMSG " + sender + " | NO matches for today | \r\n";
                     send(socket, reply.c_str(), reply.size(), 0);
                     continue;
                }
                std::string msg = message.substr(message.find(" :") + 2, message.length() - message.find(" :") - 2);
                std::string banner = "PRIVMSG " + sender + " " + "----------------------------------------------------------------------------------------------- " + "\r\n";
                send(socket, banner.c_str(), banner.size(), 0);
                for (size_t i = 0; i < matches.size(); i++)
                {
                    
                    std::string reply = "PRIVMSG " + sender + " | " + matches[i].matchTime.substr(11, 5) + " | -> "+ matches[i].league + " : | " + matches[i].homeTeam + " | vs | " + matches[i].awayTeam + " | " + "\r\n";
                    send(socket, reply.c_str(), reply.size(), 0);
                    usleep(500);
                    send(socket, banner.c_str(), banner.size(), 0);
                    usleep(500);
                }
            }
        }
        else if (valread == 0)
            throw botException("Server disconnected");
        else
            throw botException("Error in recv");
    }
}

bot::~bot()
{
    close(socket);
}

bot::botException::botException(std::string message)
{
    this->message = message;
}

const char *bot::botException::what(void) const throw()
{
    return message.c_str();
}

bot::botException::~botException() throw()
{
}
