#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <unistd.h>
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
    bool nickGiven;

public:
    user(std::string ipAddress, int socket);
    ~user();
    void setBuffer(std::string Buffer);
    std::string &getBuffer();
    void appendBuffer(std::string Buffer);
    void clearBuffer();
    void setUserName(std::string userName, std::string serverIP);
    void setNick(std::string nick, std::vector<user> users, std::string serverIP);
    bool getRegistered();
    bool getNickGiven();
    bool getPasswordCorrect();
    void setPassConfirmed(bool pass);
    std::string getUserName();
    std::string getNick();
    std::string getIpAddress();
    int getSocket();
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