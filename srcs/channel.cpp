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



const char *channel::channelException::what(void) const throw()
{
    return message.c_str();
}

channel::channelException::channelException(std::string message)
{
    this->message = message;
}


channel::channelException::~channelException() throw()
{
}
