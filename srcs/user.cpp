#include "user.hpp"

user::user(std::string ipAddress, int socket)
{
    this->registered = false;
    this->passwordCorrect = false;
    this->ipAddress = ipAddress;
    this->socket = socket;
}

user::~user()
{
    // close(socket);
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
    // this->userName = userName;
    // this->registered = true;
    // std::cout << "Username changed to " << userName << std::endl;
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
    for (int i = 0; i < users.size(); i++)
    {
        if (users[i].getNick() == nick)
        {
            throw userException("Nick already in use");
        }
    }
    this->nick = nick;
    this->registered = true;
    std::string reply = RPL_WELCOME(this->getNick(), this->getIpAddress());
    send(this->getSocket(), reply.c_str(), reply.size(), 0);
    std::string reply2 = RPL_YOURHOST(this->getNick(), this->getIpAddress());
    send(this->getSocket(), reply2.c_str(), reply2.size(), 0);
    std::string reply3 = RPL_CREATED(this->getNick(), this->getIpAddress());
    send(this->getSocket(), reply3.c_str(), reply3.size(), 0);
    std::string reply4 = RPL_MYINFO(this->getNick(), this->getIpAddress());
    send(this->getSocket(), reply4.c_str(), reply4.size(), 0);
}   