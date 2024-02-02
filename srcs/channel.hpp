#pragma once


#include <string>
#include <iostream>
#include <vector>
#include <exception>
#include "user.hpp"

class user;
class channel
{
    private:
        std::string name;
        std::vector<user> members;
        std::vector<user> operators;
        std::string password;
        std::string topic;
        bool mode; // false for public, true for invite-only
    public:
        channel(std::string name);
        ~channel();
        void setName(std::string name);
        std::string getName();
        void addMember(user member);
        void removeMember(user member);
        void addOperator(user op);
        void removeOperator(user op);
        void setPassword(std::string password);
        std::string getPassword();
        void setTopic(std::string topic);
        std::string getTopic();
        std::vector<user> getMembers();
        class channelException : public std::exception
        {
            private:
                std::string message;
            public:
                channelException(std::string message){this->message = message;}
                ~channelException();
                std::string what()
                {
                    return message.c_str();
                }
        };
};