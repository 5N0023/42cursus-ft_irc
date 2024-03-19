#include "user.hpp"

user::user(std::string ipAddress, int socket)
{
    this->nickGiven = false;
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

void user::setUserName(std::string userName, std::string serverIP)
{
    if (this->userName == "")
    {
        if(this->getNickGiven() == true)
        {
            std::string reply = RPL_WELCOME(this->getNick(), serverIP);
            send(this->getSocket(), reply.c_str(), reply.size(), 0);
            std::string reply2 = RPL_YOURHOST(this->getNick(), serverIP);
            send(this->getSocket(), reply2.c_str(), reply2.size(), 0);
            std::string reply3 = RPL_CREATED(this->getNick(), serverIP);
            send(this->getSocket(), reply3.c_str(), reply3.size(), 0);
            std::string reply4 = RPL_MYINFO(this->getNick(), serverIP);
            send(this->getSocket(), reply4.c_str(), reply4.size(), 0);
            this->registered = true;
        }
        this->userName = userName;
       
    }
    else
    {
        std::string reply = ERR_ALREADYREGISTERED(nick, serverIP);
        send(socket, reply.c_str(), reply.size(), 0);
        return;
    }
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

void user::setNick(std::string nick, std::vector<user> users, std::string serverIP)
{
    (void)serverIP;
    bool firstSet = false;
    std::string oldNick = this->nick;
    for (size_t i = 0; i < users.size(); i++)
    {
        if (users[i].getNick() == nick)
        {
            std::string reply = ERR_NICKNAMEINUSE(nick, serverIP);
            send(this->getSocket(), reply.c_str(), reply.size(), 0);
        }
    }
    if (this->nick == "")
    {
        firstSet = true;
    }
    this->nick = nick;
    this->nickGiven = true;
    if (this->getUserName() != "" && firstSet)
    {
        std::string reply = RPL_WELCOME(this->getNick(), serverIP);
        send(this->getSocket(), reply.c_str(), reply.size(), 0);
        std::string reply2 = RPL_YOURHOST(this->getNick(), serverIP);
        send(this->getSocket(), reply2.c_str(), reply2.size(), 0);
        std::string reply3 = RPL_CREATED(this->getNick(), serverIP);
        send(this->getSocket(), reply3.c_str(), reply3.size(), 0);
        std::string reply4 = RPL_MYINFO(this->getNick(), serverIP);
        send(this->getSocket(), reply4.c_str(), reply4.size(), 0);
        this->registered = true;
    }
    if (!firstSet)
    {
        std::string reply = RPL_NICKCHANGE(oldNick, this->getNick(), serverIP);
        send(this->getSocket(), reply.c_str(), reply.size(), 0);
    }
}

void user::setBuffer(std::string Buffer)
{
    this->Buffer = Buffer;
}

std::string &user::getBuffer()
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

bool user::getNickGiven()
{
    return nickGiven;
}