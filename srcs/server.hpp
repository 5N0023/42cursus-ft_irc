#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <exception>
#include "user.hpp"
#include "channel.hpp"
#include "utils.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <poll.h>
#include <unordered_map>

class server
{
    private:
        int port;
        std::string password;
        std::vector<user> users;
        std::vector<channel> channels;
        int listeningSocket;
    public:
        server(int port, std::string password);
        ~server();

        void run();
        void stop();
        void addUser(user newUser);
        void removeUser(user user);
        void addChannel(channel newChannel, user user);
        void removeChannel(channel channel);
        std::vector<user> getUsers();
        std::vector<channel> getChannels();
        user &getUserBySocket(int socket);
        class serverException : public std::exception
        {
            private:
                std::string message;
            public:
                serverException(std::string message);
                const char *what(void) const throw();
                ~serverException() throw();

        };
};