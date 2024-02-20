/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   user.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hznagui <hznagui@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 11:08:39 by hznagui           #+#    #+#             */
/*   Updated: 2024/02/20 11:08:39 by hznagui          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "user.hpp"

user::user(std::string ipAddress, int socket)
{
    this->registered = false;
    this->passwordCorrect = false;
    this->ipAddress = ipAddress;
    this->socket = socket;
    this->Buffer = "";
    this->nick = "";
    this->userName = "";
}

user::~user()
{
}

std::string user::getUserName()
{
    return userName;
}

std::string user::getNick()
{
    return nick;
}

std::string user::getIpAddress()
{
    return ipAddress;
}

int user::getSocket()
{
    return socket;
}

const char *user::userException::what(void) const throw()
{
    return message.c_str();
}

user::userException::userException(std::string message)
{
    this->message = message;
}

user::userException::~userException() throw()
{
}

void user::setUserName(std::string userName)
{
    if (this->userName == "")
    {
        std::cout << "Username set to " << userName << std::endl;
        this->userName = userName;
    }
    else
    {
        std::cerr << "Username already set with " << this->userName << "and length " << this->userName.length() << std::endl;
        std::string reply = ERR_ALREADYREGISTERED(nick, ipAddress);
        send(socket, reply.c_str(), reply.size(), 0);
        return;
    }
    std::cout << "Username changed to " << userName << std::endl;
}


bool user::getRegistered()
{
    return registered;
}

bool user::getPasswordCorrect()
{
    return passwordCorrect;
}

void user::setPassConfirmed(bool pass)
{
    passwordCorrect = pass;
}


void user::setNick(std::string nick, std::vector<user> users)
{
    bool firstSet = false;
    std::string oldNick = this->nick;
    for (size_t i = 0; i < users.size(); i++)
    {
        if (users[i].getNick() == nick)
        {
            throw userException("Nick already in use");
        }
    }
    if(this->nick == "")
    {
        firstSet = true;
    }
    this->nick = nick;
    this->registered = true;
    if (firstSet)
    {
        std::string reply = RPL_WELCOME(this->getNick(), this->getIpAddress());
        send(this->getSocket(), reply.c_str(), reply.size(), 0);
        std::string reply2 = RPL_YOURHOST(this->getNick(), this->getIpAddress());
        send(this->getSocket(), reply2.c_str(), reply2.size(), 0);
        std::string reply3 = RPL_CREATED(this->getNick(), this->getIpAddress());
        send(this->getSocket(), reply3.c_str(), reply3.size(), 0);
        std::string reply4 = RPL_MYINFO(this->getNick(), this->getIpAddress());
        send(this->getSocket(), reply4.c_str(), reply4.size(), 0);
    }
    else 
    {
        std::string reply = RPL_NICKCHANGE(oldNick, this->getNick(), this->getIpAddress());
        send(this->getSocket(), reply.c_str(), reply.size(), 0);
    }
}   

void user::setBuffer(std::string Buffer)
{
    this->Buffer = Buffer;
}

std::string user::getBuffer()
{
    return Buffer;
}

void user::appendBuffer(std::string Buffer)
{
    this->Buffer.append(Buffer);
}

void user::clearBuffer()
{
    this->Buffer.clear();
}