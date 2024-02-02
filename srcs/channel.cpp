#include "channel.hpp"


channel::channel(std::string name)
{
    this->name = name;
    this->mode = false;
    this->password = "";
}

channel::~channel()
{
}

void channel::setName(std::string name)
{
    this->name = name;
}

std::string channel::getName()
{
    return name;
}

void channel::addMember(user member)
{
    members.push_back(member);
}

void channel::setPassword(std::string password)
{
    this->password = password;
}

std::string channel::getPassword()
{
    return password;
}

std::vector<user> channel::getMembers()
{
    return members;
}

channel::channelException::~channelException()
{
}

channel::channelException::channelException(std::string message)
{
    this->message = message;
}

std::string channel::channelException::what()
{
    return message.c_str();
}
