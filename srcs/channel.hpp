/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hznagui <hznagui@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 11:08:27 by hznagui           #+#    #+#             */
/*   Updated: 2024/03/06 15:47:15 by hznagui          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once


#include <string>
#include <iostream>
#include <vector>
#include <exception>
#include "user.hpp"
#include "server.hpp"


class server;
class user;
class channel
{
    private:
        std::string name;
        std::vector<user> members;
        std::vector<user> operators;
        std::string key;
        std::string topic;
        bool has_topic;
        bool mode; // false for public, true for invite-only
    public:
        channel(std::string name);
        ~channel();
        void setMode(bool);
        bool getMode();
        bool getHasTopic();
        void setName(std::string name);
        std::string getName();
        std::string getTopic();
        void setTopic(std::string);
        void setHasTopic(bool);
        void addMember(user);
        void addOperator(user);
        bool isMember(user);
        // user isMemberstr(std::string);
        bool isoperator(user);
        // void removeMember(user member);
        // void addOperator(user op);
        // void removeOperator(user op);
        void setKey(std::string key);
        std::string getKey();
        void removeMember(user member,int);
        // void setTopic(std::string topic);
        // std::string getTopic();
        std::vector<user> &getMembers();
        class channelException : public std::exception
        {
            private:
                std::string message;
            public:
                channelException(std::string message);
                const char *what(void) const throw();
                ~channelException() throw();

        };
};