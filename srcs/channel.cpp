#include "channel.hpp"


channel::channel(std::string name)
{
    this->name = name;
    if(name.length() > 200)
        throw channelException("Channel name too long");
    if (name[0] != '#')
        throw channelException("Channel name must start with #");
    if (name.length() == 0)
        throw channelException("Channel name cannot be empty");
    this->mode = false;
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
    for (int i = 0; i < members.size(); i++)
    {
        if (members[i].getNick() == member.getNick())
        {
            throw channelException("User already in channel");
        }
    }
    members.push_back(member);
}

void channel::setKey(std::string key)
{
    this->key = key;
}

std::string channel::getKey()
{
    return key;
}

std::vector<user> &channel::getMembers()
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

void channel::removeMember(user member)
{
    int membersSize = members.size();
    for (int i = 0; i < membersSize; i++)
    {
        if (members[i].getSocket() == member.getSocket())
        {
            std::string reply = RPL_YOUPART(member.getNick(), member.getIpAddress(), member.getUserName(), name);
            members.erase(members.begin() + i);
            send(member.getSocket(), reply.c_str(), reply.size(), 0);
            return;
        }
    }
    throw channelException("User not found in channel");
}

bool channel::isMember(user member)
{
    for (int i = 0; i < members.size(); i++)
    {
        if (members[i].getNick() == member.getNick())
        {
            return true;
        }
    }
    return false;
}