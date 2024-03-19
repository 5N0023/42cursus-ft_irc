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
    std::vector<std::string> invited;
    std::vector<user> members;
    std::vector<user> operators;
    std::string key;
    std::string topic;
    bool has_topic;
    bool has_limit;
    size_t limit;
    bool has_key;      
    bool topic_strict; 
    bool mode;         
public:
    channel(std::string name);
    ~channel();
    bool getHasLimit();
    void setHasLimit(bool);
    size_t getLimit();
    void setLimit(size_t);
    void AddEraseOperator(user);
    void addInvite(user);
    void setMode(bool);
    bool getMode();
    bool getHasTopic();
    void setTopic(std::string);
    bool getHaskey();
    void setHaskey(bool);
    void setTopicStrict(bool);
    bool getTopicStrict();
    void setName(std::string name);
    std::string getName();
    std::string getTopic();
    void setHasTopic(bool);
    void addMember(user);
    void addOperator(user);
    bool isMember(user);
    bool isoperator(user);
    void setKey(std::string);

    bool isInvited(user);
    bool getInviteOnly();
    void eraseInvited(user);
    std::string getKey();
    void removeMember(user member, int);
    std::vector<user> &getMembers();
    std::vector<user> &getOperators();
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