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
        std::string Buffer;
        std::string userName;
        std::string nick;
        std::string ipAddress;
        int socket;
        bool registered;
        bool passwordCorrect;
    public:
        user(std::string ipAddress, int socket);
        ~user();
        void setBuffer(std::string Buffer);
        std::string getBuffer();
        void appendBuffer(std::string Buffer);
        void clearBuffer();
        void setUserName(std::string userName);
        void setNick(std::string nick, std::vector<user> users);
        bool getRegistered();
        bool getPasswordCorrect();
        void setPassConfirmed(bool pass);
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