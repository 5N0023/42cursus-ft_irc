#pragma once


#include <string>
#include <iostream>
#include <vector>
#include <exception>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <poll.h>
#include <map>
#include <fcntl.h>
#include "bot.hpp"
#include "utils.hpp"

class bot
{
    private:
        std::string server;
        int serverPort;
        std::string nick;
        std::string user;
        std::string realname;
        std::vector<MatchInfo> matches;
        int socket;
    public:
        bot(std::string server, int port, std::string nick, std::string user, std::string realname);
        ~bot();
        void connectToServer();
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
};
