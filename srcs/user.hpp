#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <unistd.h> // for close()
#include <poll.h>
#include <exception>
#include "channel.hpp"

class channel;
class user
{
private:
    std::string userName;
    std::string nick;
    std::string ipAddress;
    int socket;
    bool registered;
public:
    user(std::string nick, std::string ipAddress, int socket, std::vector<user> users);
    ~user();
    void setUserName(std::string userName, std::vector<user> users);
    // void setNick(std::string nick);
    // void setIpAddress(std::string ipAddress);
    // void setSocket(int socket);
    std::string getUserName();
    std::string getNick();
    std::string getIpAddress();
    // void changeNick(std::string newNick, std::vector<user> users);
    int getSocket();
    // void sendPrivateMessage(std::string message, user recipient);
    // void sendChannelMessage(std::string message, channel channel);
    class userException : public std::exception
        {
            private:
                std::string message;
            public:
                userException(std::string message);
                const char *what(void) const throw();
                ~userException() throw();

        };
};