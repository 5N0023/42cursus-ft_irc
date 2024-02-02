#include "user.hpp"

user::user(std::string nick, std::string ipAddress, int socket, std::vector<user> users)
{
    this->registered = false;
    this->nick = nick;
    this->ipAddress = ipAddress;
    this->socket = socket;
    int usersSize = users.size();
    for (int i = 0; i < usersSize; i++)
    {
        if (users[i].getUserName() == userName)
        {
            throw userException("User already exists");
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

void user::setUserName(std::string userName, std::vector<user> users)
{
    int usersSize = users.size();
    for (int i = 0; i < usersSize; i++)
    {
        if (users[i].getUserName() == userName)
        {
            throw userException("User already exists");
        }
    }
    this->userName = userName;
    this->registered = true;
    std::cout << "Username changed to " << userName << std::endl;
}
