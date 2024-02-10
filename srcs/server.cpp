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
    if (fcntl(listeningSocket, F_SETFL, O_NONBLOCK) < 0)
    {
        throw serverException("Error setting socket to non-blocking\n");
    }

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
                    try {
                    struct sockaddr_in clientAddr;
                    socklen_t clientAddrLen = sizeof(clientAddr);
                    int clientSocket = accept(listeningSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
                    if (clientSocket < 0)
                    {
                        throw serverException("Error accepting connection\n");
                    }
                    if  (fcntl(clientSocket, F_SETFL, O_NONBLOCK) < 0)
                    {
                        throw serverException("Error setting socket to non-blocking\n");
                    }

                    // Store client IP address
                    clientIPs[clientSocket] = inet_ntoa(clientAddr.sin_addr);

                    struct pollfd clientFd;
                    clientFd.fd = clientSocket;
                    clientFd.events = POLLIN;
                    fds.push_back(clientFd);
                    }
                    catch (server::serverException &e)
                    {
                        std::cerr << "Error: " << e.what() << "\n";
                    }
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
                        std::string reply = ERR_INPUTTOOLONG(clientIPs[fds[i].fd], serverIP);
                        send(fds[i].fd, reply.c_str(), reply.size(), 0);
                    }
                    else
                    {
                        if (this->getUserBySocket(fds[i].fd) == -1)
                        {

                            try {
                                    this->addUser(user(clientIPs[fds[i].fd], fds[i].fd));
                                    std::cout << "New user connected from " << clientIPs[fds[i].fd] << std::endl;
                                }
                            catch (user::userException &e)
                                {
                                    std::cerr << "Error: " << e.what() << "\n";
                                }
                        }
                        std::string sBuffer = std::string(buffer, bytesRead);
                        if (sBuffer.find('\n') == std::string::npos)
                        {
                            if (this->getUserBySocket(fds[i].fd) != -1)
                                users[this->getUserBySocket(fds[i].fd)].appendBuffer(sBuffer);
                            continue;
                        }
                        else
                        {
                            if (this->getUserBySocket(fds[i].fd) != -1)
                            {
                                users[this->getUserBySocket(fds[i].fd)].appendBuffer(sBuffer);
                                sBuffer = users[this->getUserBySocket(fds[i].fd)].getBuffer();
                                users[this->getUserBySocket(fds[i].fd)].clearBuffer();
                            }
                        }
                        if (sBuffer[sBuffer.size() - 1] == '\n')
                            sBuffer = sBuffer.substr(0, sBuffer.size() - 2);
                        else
                            sBuffer = sBuffer.substr(0, sBuffer.size() - 3);
                        std::cerr << "sBuffer: " << sBuffer << std::endl;
                        try {
                            if (sBuffer.substr(0, 4) != "PASS" && this->getUserBySocket(fds[i].fd) != -1 && users[this->getUserBySocket(fds[i].fd)].getPasswordCorrect() == false)
                            {
                                std::string reply = ERR_NEEDPASS(clientIPs[fds[i].fd], serverIP);
                                send(fds[i].fd, reply.c_str(), reply.size(), 0);
                                continue;
                            }
                            if (sBuffer.substr(0, 4) != "NICK" && this->getUserBySocket(fds[i].fd) != -1 && users[this->getUserBySocket(fds[i].fd)].getRegistered() == false && users[this->getUserBySocket(fds[i].fd)].getPasswordCorrect() == true)
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
                        if (sBuffer.substr(0, 4) == "PASS")
                        {
                            try
                            {
                                std::vector<std::string> args = splitCommand(sBuffer);
                                if (args.size() < 2)
                                {
                                    std::string reply = ERR_NEEDMOREPARAMS(clientIPs[fds[i].fd], serverIP, "PASS");
                                    send(fds[i].fd, reply.c_str(), reply.size(), 0);
                                    continue;
                                }
                                std::cerr << "args[1] which is password : " << args[1] << std::endl;
                                if (args[1] != password)
                                {
                                    std::string reply = ERR_PASSWDMISMATCH(clientIPs[fds[i].fd], serverIP);
                                    send(fds[i].fd, reply.c_str(), reply.size(), 0);
                                }
                                else
                                {
                                    int user = this->getUserBySocket(fds[i].fd);
                                    if (user != -1)
                                        users[user].setPassConfirmed(true);
                                }
                            }
                            catch (user::userException &e)
                            {
                                std::cerr << "Error: " << e.what() << "\n";
                            }
                        }
                        else if (sBuffer.substr(0, 4) == "QUIT")
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
                        else if (sBuffer.substr(0, 4) == "NICK")
                        {
                            std::vector<std::string> args = splitCommand(sBuffer);
                            if (args.size() < 2)
                            {
                                std::string reply = ERR_NONICKNAMEGIVEN(clientIPs[fds[i].fd], serverIP);
                                send(fds[i].fd, reply.c_str(), reply.size(), 0);
                                continue;
                            }
                            try {
                                int user = this->getUserBySocket(fds[i].fd);
                                if (user != -1)
                                    users[user].setNick(args[1], users);
                            }
                            catch (user::userException &e)
                            {
                                std::cerr << "Error NICK: " << e.what() << "\n";
                                std::string reply = ERR_NICKNAMEINUSE(args[1], clientIPs[fds[i].fd]);
                                send(fds[i].fd, reply.c_str(), reply.size(), 0);
                            }
                        }
                        else if (sBuffer.substr(0, 4) == "USER")
                        {
                            std::cerr << "USER command : " << sBuffer << std::endl;
                            std::vector<std::string> args = splitCommand(sBuffer);
                            if (args.size() < 5)
                            {
                                std::string reply = ERR_NEEDMOREPARAMS(clientIPs[fds[i].fd], serverIP, "USER");
                                send(fds[i].fd, reply.c_str(), reply.size(), 0);
                                continue;
                            }
                            try {
                                int user = this->getUserBySocket(fds[i].fd);
                                for (int i = 0; i < args[1].size(); i++)
                                {
                                    if (args[1][i] == ' ')
                                    {
                                        args[1] = args[1].substr(0, i);
                                        break;
                                    }
                                }
                                if (user != -1)
                                    users[user].setUserName(args[1]);
                            }
                            catch (user::userException &e)
                            {
                                std::cerr << "Error USER: " << e.what() << "\n";
                            }
                        }
                        else if (sBuffer.substr(0, 4) == "JOIN")
                        {
                            try {
                                int joinedUser = this->getUserBySocket(fds[i].fd);
                                std::string channelName = sBuffer.substr(5, sBuffer.size() - 1);
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
                        else if (sBuffer.substr(0, 4) == "PART")
                        {
                            try {
                                int partUser = this->getUserBySocket(fds[i].fd);
                                if (partUser == -1)
                                    throw serverException("User not found");
                                std::string channelName = sBuffer.substr(5, sBuffer.size() - 1);
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
                        else if (sBuffer.substr(0, 7) == "PRIVMSG")
                        {
                            std::string receiver = sBuffer.substr(8, sBuffer.size() - 1);
                            for (int i = 0; i < receiver.size(); i++)
                            {
                                if (receiver[i] == ' ')
                                {
                                    receiver = receiver.substr(0, i);
                                    break;
                                }
                            }
                            std::string message = sBuffer.substr(8, sBuffer.size() - 1);
                            message = message.substr(receiver.size() + 1, message.size() - 1);
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
                            if (message[0] == ':')
                            {
                                message = message.substr(1, message.size() - 1);
                            }
                            try {
                                int sender = this->getUserBySocket(fds[i].fd);
                                if (sender == -1)
                                    throw serverException("User not found");
                                for (int i = 0; i < receivers.size(); i++)
                                {
                                    if (receivers[i][0] == '#' || receivers[i][0] == '&')
                                        this->prvmsgchannel(users[sender], receivers[i], message);
                                    else 
                                        this->prvmsg(users[sender], receivers[i], message);
                                }
                            }
                            catch (server::serverException &e)
                            {
                                std::cerr << "Error PRIVMSG: " << e.what() << "\n";
                            }
                        }
                        else if (sBuffer.substr(0, 4) != "PONG")
                        {
                            std::string command = sBuffer.substr(0, sBuffer.size() - 1);
                            if (command[command.size() - 1] == '\n')
                                command = command.substr(0, command.size() - 2);
                            else
                                command = command.substr(0, command.size() - 3);
                            for (int i = 0; i < command.size(); i++)
                            {
                                if (command[i] == ' ')
                                {
                                    command = command.substr(0, i);
                                    break;
                                }
                            }
                            int user = this->getUserBySocket(fds[i].fd);
                            if (user != -1)
                            {
                                std::string reply = ERR_UNKNOWNCOMMAND(users[user].getNick(), serverIP, command);
                                send(fds[i].fd, reply.c_str(), reply.size(), 0);
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
    bool userExists = false;
    for (int j = 0; j < users.size(); j++)
    {
        if (users[j].getNick() == receiver)
        {
            userExists = true;
            reply = PRIVMSG_FORMAT(sender.getNick(), sender.getUserName(), sender.getIpAddress(), receiver, message);
            std::cerr << "reply: " << reply << std::endl;
            send(users[j].getSocket(), reply.c_str(), reply.size(), 0);
        }
    }
    if (userExists == false)
    {
        reply = ERR_NOSUCHNICK(serverIP, receiver);
        send(sender.getSocket(), reply.c_str(), reply.size(), 0);
    }
}
void server::prvmsgchannel(user sender, std::string receiverchannel, std::string message)
{
    std::string reply;
    bool channelExists = false;
    for (int j = 0; j < channels.size(); j++)
    {
        if (channels[j].getName() == receiverchannel)
        {
            channelExists = true;
            if (channels[j].isMember(sender) == false)
            {
                reply = ERR_CANNOTSENDTOCHAN(serverIP, receiverchannel);
                send(sender.getSocket(), reply.c_str(), reply.size(), 0);
                return;
            }
            std::vector<user> members = channels[j].getMembers();
            for (int k = 0; k < members.size(); k++)
            {
                if (members[k].getNick() == sender.getNick())
                    continue;
                reply = PRIVMSG_FORMAT(sender.getNick(), sender.getUserName(), sender.getIpAddress(), receiverchannel, message);
                send(members[k].getSocket(), reply.c_str(), reply.size(), 0);
            }
        }
    }
    if (channelExists == false)
    {
        reply = ERR_NOSUCHCHANNEL(serverIP, receiverchannel);
        send(sender.getSocket(), reply.c_str(), reply.size(), 0);
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