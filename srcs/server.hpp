#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <exception>
#include "user.hpp"
#include "channel.hpp"

class server
{
    private:
        int port;
        std::string password;
        std::vector<user> users;
        std::vector<channel> channels;
    public:
        server(int port, std::string password);
        ~server();

        void run();
        void stop();
        void addUser(user newUser);
        void removeUser(user user);
        void addChannel(channel newChannel);
        void removeChannel(channel channel);
        std::vector<user> getUsers();
        std::vector<channel> getChannels();
        class serverException : public std::exception
        {
            private:
                std::string message;
            public:
                serverException(std::string message){this->message = message;}
                ~serverException();
                std::string what()
                {
                    return message.c_str();
                }
        };
};