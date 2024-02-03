#include "user.hpp"

user::user(std::string nick, std::string ipAddress, int socket, std::vector<user> users)
{
    this->registered = false;
    this->nick = nick;
    this->ipAddress = ipAddress;
    this->socket = socket;
    int usersSize = users.size();
    if (nick == "")
    {
        throw userException("Nick cannot be empty");
    }
    if (nick.length() > 9)
    {
        throw userException("Nick cannot be longer than 9 characters");
    }
    for (int i = 0; i < usersSize; i++)
    {
        if (users[i].getNick() == nick)
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
    this->userName = userName;
    this->registered = true;
    std::cout << "Username changed to " << userName << std::endl;
}
