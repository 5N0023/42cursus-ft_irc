#include "user.hpp"

user::user(std::string userName, std::string nick, std::string ipAddress, int socket, std::vector<user> users)
{
    this->userName = userName;
    this->nick = nick;
    this->ipAddress = ipAddress;
    this->socket = socket;
    int usersSize = users.size();
    for (int i = 0; i < usersSize; i++)
    {
        if (users[i].getUserName() == userName)
        {
            throw userException("Username already in use");
        }
    }
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