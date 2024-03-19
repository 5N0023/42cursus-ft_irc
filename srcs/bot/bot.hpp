#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h> // For setting socket non-blocking
#include <iostream>
#include <string>
#include "utils.hpp"

#define CONNECTION_PORT 6697

class bot
{
private:
    std::string server;
    std::string serverPassword;
    int serverPort;
    std::string nick;
    std::string user;
    std::string realname;
    std::vector<MatchInfo> matches;
    int socket;

public:
    bot(std::string server, int serverPort, std::string nick, std::string user, std::string realname, std::string serverPassword);

    ~bot();
    void connectToServer();
    void listenToServerAndRespond();
    // void disconnectFromServer();
    // void sendToServer(std::string message);
    // void joinChannel(std::string channel);
    // void partChannel(std::string channel);
    // void privmsg(std::string receiver, std::string message);
    // void run();
    // void stop();
    // class botException : public std::exception
    // {
    //     private:
    //         std::string message;
    //     public:
    //         botException(std::string message);
    //         const char *what(void) const throw();
    //         ~botException() throw();

    // };

    class botException : public std::exception
    {
    private:
        std::string message;

    public:
        botException(std::string message);
        const char *what(void) const throw();
        ~botException() throw();
    };
};
