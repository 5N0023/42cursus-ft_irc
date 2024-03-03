/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hznagui <hznagui@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 11:08:22 by hznagui           #+#    #+#             */
/*   Updated: 2024/03/03 18:19:42 by hznagui          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "channel.hpp"

std::string channel::getTopic(){return topic;}
void channel::setTopic(std::string Topic){topic=Topic;}

bool  channel::getHasTopic(){return has_topic;}


channel::channel(std::string name)
{
    this->name = name;
    if(name.length() > 200)
        throw channelException("Channel name too long");
    if (name[0] != '#')
        throw channelException("Channel name must start with #");
    if (name.length() == 0)
        throw channelException("Channel name cannot be empty");
    this->has_topic = false;
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
    for (size_t i = 0; i < members.size(); i++)
    {
        if (members[i].getNick() == member.getNick())
        {
            throw channelException("User already in channel");
        }
    }
    members.push_back(member);
}
void channel::addOperator(user member)
{
    for (size_t i = 0; i < operators.size(); i++)
    {
        if (members[i].getNick() == member.getNick())
            return;
    }
    operators.push_back(member);
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

void channel::removeMember(user member,int index)
{
    if (member.getSocket() == -1 && member.getIpAddress() == "error")
        return;
    int membersSize = members.size();
    for (int i = 0; i < membersSize; i++)
    {
        if (members[i].getSocket() == member.getSocket())
        {
            if (index == 1)
                {std::string reply = RPL_YOUPART(member.getNick(), member.getIpAddress(), member.getUserName(), name);
                send(member.getSocket(), reply.c_str(), reply.size(), 0);}
            members.erase(members.begin() + i);
            return;
        }
    }
    throw channelException("User not found in channel");
}

bool channel::isMember(user member)
{
    for (size_t i = 0; i < members.size(); i++)
    {
        if (members[i].getNick() == member.getNick())
        {
            return true;
        }
    }
    return false;
}

bool channel::isoperator(user operat)
{
    for (size_t i = 0; i < operators.size(); i++)
    {
        if (operators[i].getNick() == operat.getNick())
            return true;
    }
   
    return false;
}