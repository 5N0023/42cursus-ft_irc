#include "server.hpp"

server::server(int port, std::string password): port(port), password(password)
{
    listeningSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listeningSocket < 0)
    {
        throw serverException("Error creating socket\n");
    }
    int opt = 1;
    if (setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        throw serverException("Error setting socket options\n");
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(listeningSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        throw serverException("Error binding socket\n");
    }

    // Listen on socket
    if (listen(listeningSocket, 1024) < 0)
    {
        throw serverException("Error listening on socket\n");
    }
}

server::~server()
{
    close(listeningSocket);
}

void server::run()
{
    std::vector<struct pollfd> fds;
    struct pollfd listenFd;
    listenFd.fd = listeningSocket;
    listenFd.events = POLLIN;
    fds.push_back(listenFd);
    // get the ip address of the server
    serverIP = "0.0.0.0";


    // Map to store client IP addresses
    std::map<int, std::string> clientIPs;

    while (true)
    {
        int ret = poll(fds.data(), fds.size(), -1); // -1 means wait indefinitely
        if (ret < 0)
        {
            throw serverException("Error on poll()\n");
        }

        for (size_t i = 0; i < fds.size(); ++i)
        {
            if (fds[i].revents & POLLIN)
            {
                if (fds[i].fd == listeningSocket)
                {
                    // Accept new connection
                    struct sockaddr_in clientAddr;
                    socklen_t clientAddrLen = sizeof(clientAddr);
                    int clientSocket = accept(listeningSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
                    if (clientSocket < 0)
                    {
                        throw serverException("Error accepting connection\n");
                    }

                    // Store client IP address
                    clientIPs[clientSocket] = inet_ntoa(clientAddr.sin_addr);

                    struct pollfd clientFd;
                    clientFd.fd = clientSocket;
                    clientFd.events = POLLIN;
                    fds.push_back(clientFd);
                }
                else
                {
                    // Handle data from client
                    char buffer[1024];
                    int bytesRead = recv(fds[i].fd, buffer, sizeof(buffer), 0);
                    if (bytesRead <= 0)
                    {
                        // Connection closed or error
                        close(fds[i].fd);
                        clientIPs.erase(fds[i].fd); // Remove IP from map
                        fds.erase(fds.begin() + i);
                        --i; // Adjust index after erase
                        try{
                            if (this->getUserBySocket(fds[i].fd) != -1)
                            {
                            this->removeUser(users[this->getUserBySocket(fds[i].fd)]);
                            }
                        }
                        catch (server::serverException &e)
                        {
                            std::cerr << "Error: " << e.what() << "\n";
                        
                        }
                    }
                    else if (bytesRead > 512)
                    {
                        std::cerr << "Received too many bytes\n";
                    }
                    else
                    {
                        if (this->getUserBySocket(fds[i].fd) == -1)
                        {

                            try {
                                    std::cerr << "new user here with socket " << fds[i].fd << std::endl;
                                    this->addUser(user(clientIPs[fds[i].fd], fds[i].fd));
                                }
                            catch (user::userException &e)
                                {
                                    std::cerr << "Error: " << e.what() << "\n";
                                }
                                std::cerr << "new user here" << std::endl;
                        }
                        try {
                            if (memcmp(buffer, "PASS", 4) != 0 && this->getUserBySocket(fds[i].fd) != -1 &&  users[this->getUserBySocket(fds[i].fd)].getPasswordCorrect() == false)
                            {
                                std::string reply = ERR_NEEDPASS(clientIPs[fds[i].fd], serverIP);
                                send(fds[i].fd, reply.c_str(), reply.size(), 0);
                                continue;
                            }
                            if (memcmp(buffer, "NICK", 4) != 0 && this->getUserBySocket(fds[i].fd) != -1 && users[this->getUserBySocket(fds[i].fd)].getRegistered() == false && users[this->getUserBySocket(fds[i].fd)].getPasswordCorrect() == true)
                            {
                                std::string reply = ERR_NEEDNICK(clientIPs[fds[i].fd], serverIP);
                                send(fds[i].fd, reply.c_str(), reply.size(), 0);
                                continue;
                            }
                        }
                        catch (user::userException &e)
                        {
                            std::cerr << "Error: " << e.what() << "\n";
                        }
                        if (memcmp(buffer, "PASS", 4) == 0)
                        {
                            try{
                                std::string pass = std::string(buffer + 5, bytesRead - 5);
                                if (pass[pass.size() - 1] == '\n')
                                    pass = pass.substr(0, pass.size() - 1);
                                else 
                                    pass = pass.substr(0, pass.size() - 2);
                                    
                                if (pass != password)
                                {
                                    std::string reply = ERR_PASSWDMISMATCH(clientIPs[fds[i].fd], serverIP);
                                    send(fds[i].fd, reply.c_str(), reply.size(), 0);
                                }
                                else
                                {
                                    std::cerr << "Password correct" << std::endl;
                                    int user = this->getUserBySocket(fds[i].fd);
                                    if (user != -1)
                                    {
                                        std::cerr << "user with socket " << fds[i].fd << " has password confirmed\n";
                                        users[user].setPassConfirmed(true);
                                    }
                                }
                            }
                            catch (user::userException &e)
                            {
                                std::cerr << "Error: " << e.what() << "\n";
                            }
                        }
                        if (memcmp(buffer, "QUIT", 4) == 0)
                        {
                            try {
                                int user = this->getUserBySocket(fds[i].fd);
                                close(fds[i].fd);
                                clientIPs.erase(fds[i].fd);
                                fds.erase(fds.begin() + i);
                                --i;
                                if (user != -1)
                                    this->removeUser(users[user]);
                            }
                            catch (server::serverException &e)
                            {
                                std::cerr << "Error: " << e.what() << "\n";
                            }
                        }
                        if (memcmp(buffer, "NICK", 4) == 0)
                        {
                            std::cerr << "NICK command received" << std::endl;
                            std::string nick = std::string(buffer + 5, bytesRead - 5);
                            if (nick[nick.size() - 1] == '\n')
                                nick = nick.substr(0, nick.size() - 1);
                            else
                                nick = nick.substr(0, nick.size() - 2);
                            try {
                                int user = this->getUserBySocket(fds[i].fd);
                                if (user != -1)
                                    users[user].setNick(nick, users);
                                std::cerr << "Nick set to " << nick << std::endl;
                            }
                            catch (user::userException &e)
                            {
                                std::cerr << "Error NICK: " << e.what() << "\n";
                                std::string reply = ERR_NICKNAMEINUSE(nick, clientIPs[fds[i].fd]);
                                send(fds[i].fd, reply.c_str(), reply.size(), 0);
                            }
                        }
                        if (memcmp(buffer, "USER", 4) == 0)
                        {
                            std::string userName = std::string(buffer + 5, bytesRead - 5);
                            if (userName[userName.size() - 1] == '\n')
                                userName = userName.substr(0, userName.size() - 1);
                            else
                                userName = userName.substr(0, userName.size() - 2);
                            try {
                                // i have no idea why this is not working
                                int user = this->getUserBySocket(fds[i].fd);
                                if (user != -1)
                                    users[user].setUserName(userName);
                            }
                            catch (user::userException &e)
                            {
                                std::cerr << "Error USER: " << e.what() << "\n";
                            }
                        }
                        if (memcmp(buffer, "JOIN", 4) == 0)
                        {
                            try {
                                int joinedUser = this->getUserBySocket(fds[i].fd);
                                std::string channelName = std::string(buffer + 5, bytesRead - 5);
                                if (channelName[channelName.size() - 1] == '\n')
                                    channelName = channelName.substr(0, channelName.size() - 1);
                                else
                                    channelName = channelName.substr(0, channelName.size() - 2);
                                for (int i = 0; i < channelName.size(); i++)
                                {
                                    if (channelName[i] == ' ' || channelName[i] == '\n')
                                    {
                                        channelName = channelName.substr(0, i);
                                        break;
                                    }
                                }
                                channel newChannel(channelName);
                                if (joinedUser != -1)
                                    this->addChannel(newChannel, users[joinedUser]);
                            }
                            catch (channel::channelException &e)
                            {
                                std::cerr << "Error JOIN: " << e.what() << "\n";
                            }
                        }
                        if (memcmp(buffer, "PART", 4) == 0)
                        {
                            try {
                                int partUser = this->getUserBySocket(fds[i].fd);
                                if (partUser == -1)
                                    throw serverException("User not found");
                                std::string channelName = std::string(buffer + 5, bytesRead - 5);
                                if (channelName[channelName.size() - 1] == '\n')
                                    channelName = channelName.substr(0, channelName.size() - 1);
                                else
                                    channelName = channelName.substr(0, channelName.size() - 2);
                                for (int i = 0; i < channelName.size(); i++)
                                {
                                    if (channelName[i] == ' ' || channelName[i] == ',')
                                    {
                                        channelName = channelName.substr(0, i);
                                        break;
                                    }
                                }
                                for (int i = 0; i < channels.size(); i++)
                                {
                                    if (channels[i].getName() == channelName)
                                    {
                                        this->channels[i].removeMember(users[partUser]);
                                        if (channels[i].getMembers().size() == 0)
                                        {
                                            this->removeChannel(channels[i]);
                                            i--;
                                        }
                                    }
                                }
                            }
                            catch (channel::channelException &e)
                            {
                                std::cerr << "Error PART: " << e.what() << "\n";
                            }
                        }
                        if (memcmp(buffer, "PRIVMSG", 7) == 0)
                        {
                            std::string receiver = std::string(buffer + 8, bytesRead - 8);
                            for (int i = 0; i < receiver.size(); i++)
                            {
                                if (receiver[i] == ' ')
                                {
                                    receiver = receiver.substr(0, i);
                                    break;
                                }
                            }
                            std::string message = std::string(buffer + 8, bytesRead - 8);
                            if (message[message.size() - 1] == '\n')
                                message = message.substr(0, message.size() - 1);
                            else
                                message = message.substr(0, message.size() - 2);
                            message = message.substr(receiver.size() + 1, message.size() - receiver.size() - 3);
                            if (message[0] == ':')
                            {
                                message = message.substr(1, message.size() - 2);
                            }
                            try {
                                int sender = this->getUserBySocket(fds[i].fd);
                                if (sender == -1)
                                    throw serverException("User not found");
                                if (receiver[0] == '#')
                                    this->prvmsgchannel(users[sender], receiver, message);
                                else 
                                    this->prvmsg(users[sender], receiver, message);
                            }
                            catch (server::serverException &e)
                            {
                                std::cerr << "Error PRIVMSG: " << e.what() << "\n";
                            }
                        }
                    }
                }
            }
        }
    }
}


void server::stop()
{
    close(listeningSocket);
}

void server::addUser(user newUser)
{
    int usersSize = users.size();
    for (int i = 0; i < usersSize; i++)
    {
        if (users[i].getNick() == newUser.getNick())
        {
            return;
        }
    }
    users.push_back(newUser);
    
}

void server::removeUser(user user)
{
    int usersSize = users.size();
    for (int i = 0; i < usersSize; i++)
    {
        if (users[i].getUserName() == user.getUserName())
        {
            users.erase(users.begin() + i);
            return;
        }
    }
    throw serverException("User not found in server\n");
}


void server::addChannel(channel newChannel, user user)
{
    bool channelExists = false;
    if (newChannel.getName()[0] != '#')
    {
        std::string reply = ERR_BADCHANNELNAME(user.getNick(), user.getIpAddress(), newChannel.getName());
        send(user.getSocket(), reply.c_str(), reply.size(), 0);
        return;
    }
    int channelsSize = channels.size();
    for (int i = 0; i < channelsSize; i++)
    {
        if (channels[i].getName() == newChannel.getName())
        {
            std::string reply = RPL_JOIN(user.getNick(), user.getUserName(), newChannel.getName(), user.getIpAddress());
            send(user.getSocket(), reply.c_str(), reply.size(), 0);
            channelExists = true;
            channels[i].addMember(user);
            break;
        }
    }
    if (channelExists)
    {
        return;
    }
    newChannel.addMember(user);
    channels.push_back(newChannel);
    std::string reply = RPL_JOIN(user.getNick(), user.getUserName(), newChannel.getName(), user.getIpAddress());
    send(user.getSocket(), reply.c_str(), reply.size(), 0);

}

int server::getUserBySocket(int socket)
{
    int usersSize = users.size();
    for (int i = 0; i < usersSize; i++)
    {
        if (users[i].getSocket() == socket)
        {
            return i;
        }
    }
    return -1;
}


std::vector<user> &server::getUsers()
{
    return users;
}

std::vector<channel> &server::getChannels()
{
    return channels;
}

const char *server::serverException::what(void) const throw()
{
    return message.c_str();
}

server::serverException::serverException(std::string message)
{
    this->message = message;
}

server::serverException::~serverException() throw()
{
}

void server::prvmsg(user sender, std::string receiver, std::string message)
{
    std::vector<user> users = this->getUsers();
    std::string reply;
    std::vector<std::string> receivers;
    for (int i = 0; i < receiver.size(); i++)
    {
        if (receiver[i] == ',')
        {
            receivers.push_back(receiver.substr(0, i));
            receiver = receiver.substr(i + 1, receiver.size() - i - 1);
            i = 0;
        }
        else if (i == receiver.size() - 1)
        {
            receivers.push_back(receiver);
        }
    }
    for (int i = 0; i < receivers.size(); i++)
    {
        for (int j = 0; j < users.size(); j++)
        {
            if (users[j].getNick() == receivers[i])
            {
                reply = PRIVMSG_FORMAT(sender.getNick(), sender.getUserName(), sender.getIpAddress(), receivers[i], message);
                send(users[j].getSocket(), reply.c_str(), reply.size(), 0);
            }
        }
    }
}
void server::prvmsgchannel(user sender, std::string receiverschannels, std::string message)
{
    std::string reply;
    std::vector<std::string> receivers;
    for (int i = 0; i < receiverschannels.size(); i++)
    {
        if (receiverschannels[i] == ',')
        {
            receivers.push_back(receiverschannels.substr(0, i));
            receiverschannels = receiverschannels.substr(i + 1, receiverschannels.size() - i - 1);
            i = 0;
        }
        else if (i == receiverschannels.size() - 1)
        {
            receivers.push_back(receiverschannels);
        }
    }

    for (int i = 0; i < receivers.size(); i++)
    {
        for (int j = 0; j < channels.size(); j++)
        {
            if (channels[j].getName() == receivers[i])
            {
                std::vector<user> members = channels[j].getMembers();
                for (int k = 0; k < members.size(); k++)
                {
                    if (members[k].getNick() == sender.getNick())
                        continue;
                    reply = PRIVMSG_FORMAT(sender.getNick(), sender.getUserName(), sender.getIpAddress(), receivers[i], message);
                    send(members[k].getSocket(), reply.c_str(), reply.size(), 0);
                }
            }
        }
    }
}

 void server::removeChannel(channel channel)
 {
     int channelsSize = channels.size();
     for (int i = 0; i < channelsSize; i++)
     {
         if (channels[i].getName() == channel.getName())
         {
             channels.erase(channels.begin() + i);
             return;
         }
     }
    throw serverException("Channel not found");
 }