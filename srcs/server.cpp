#include "server.hpp"

server::server(int Port, std::string password) : password(password)
{
    port = Port;
    serverIP = getLocalIP();
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

    listenFd.fd = listeningSocket;
    listenFd.events = POLLIN;
    fds.push_back(listenFd);

    std::cerr << "serverIP: " << serverIP << std::endl;
    std::string sBufferAll;

    while (true)
    {
        int ret = poll(fds.data(), fds.size(), -1);
        if (ret < 0)
        {
            throw serverException("Error on poll()\n");
        }

        for (size_t i = 0; i < fds.size(); ++i)
        {
            if (fds[i].revents & POLLIN)
            {
                if (fds[i].fd == listeningSocket)
                    newConnections();

                else
                {
                    char buffer[1024];
                    int bytesRead = recv(fds[i].fd, buffer, sizeof(buffer), 0);
                    
                    if (bytesRead <= 0)
                        disconnecting(i);

                    else if (bytesRead > 512)
                    {
                        std::string reply = ERR_INPUTTOOLONG(clientIPs[fds[i].fd], serverIP);
                        send(fds[i].fd, reply.c_str(), reply.size(), 0);
                    }
                    else
                    {
                        sBufferAll.clear();
                        sBufferAll = std::string(buffer, bytesRead);
                        std::vector<std::string> allCommmands = split(sBufferAll, '\n');
                        for (size_t d = 0; d < allCommmands.size(); d++)
                        {
                            std::string sBuffer = allCommmands[d];
                            sBuffer += '\n';
                            if (d == allCommmands.size() - 1 && sBufferAll[sBufferAll.size() - 1] != '\n')
                                sBuffer = sBuffer.substr(0, sBuffer.size() - 1);
                            if (sBuffer.find('\n') == std::string::npos)
                            {
                                if (getUserBySocket(fds[i].fd) != -1)
                                    users[getUserBySocket(fds[i].fd)].appendBuffer(sBuffer);
                                continue;
                            }
                            else
                            {
                                if (getUserBySocket(fds[i].fd) != -1)
                                {
                                    users[getUserBySocket(fds[i].fd)].appendBuffer(sBuffer);
                                    sBuffer = users[getUserBySocket(fds[i].fd)].getBuffer();
                                    users[getUserBySocket(fds[i].fd)].clearBuffer();
                                }
                            }
                            sBuffer = sBuffer.substr(0, sBuffer.find('\n'));
                            sBuffer = sBuffer.substr(0, sBuffer.find('\r'));
                            std::cout << "server received: " << sBuffer << std::endl;
                            if (sBuffer.substr(0, 4) == "QUIT")
                            {
                                quit(fds[i].fd, i);
                                continue;
                            }
                            try
                            {
                                if (sBuffer.substr(0, 4) != "PASS" && getUserBySocket(fds[i].fd) != -1 && users[getUserBySocket(fds[i].fd)].getPasswordCorrect() == false)
                                {
                                    std::string reply = ERR_NEEDPASS(clientIPs[fds[i].fd], serverIP);
                                    send(fds[i].fd, reply.c_str(), reply.size(), 0);
                                    continue;
                                }
                                if ((sBuffer.substr(0, 4) != "NICK" && sBuffer.substr(0, 4) != "USER") && getUserBySocket(fds[i].fd) != -1 && users[getUserBySocket(fds[i].fd)].getNickGiven() == false && users[getUserBySocket(fds[i].fd)].getPasswordCorrect() == true)
                                {
                                    std::string reply = ERR_NOTREGISTERED(clientIPs[fds[i].fd], serverIP);
                                    send(fds[i].fd, reply.c_str(), reply.size(), 0);
                                    continue;
                                }
                            }
                            catch (std::exception &e)
                            {
                                std::cerr << "Error: " << e.what() << "\n";
                            }

                            if (sBuffer.substr(0, 4) == "PASS")
                                pass(fds[i].fd, sBuffer, clientIPs[fds[i].fd]);

                            else if (sBuffer.substr(0, 4) == "NICK")
                                nick(fds[i].fd, sBuffer, clientIPs[fds[i].fd]);

                            else if (sBuffer.substr(0, 4) == "USER")
                                usercmd(fds[i].fd, sBuffer, clientIPs[fds[i].fd]);

                            else if (sBuffer.substr(0, 4) == "JOIN")
                                join(fds[i].fd, sBuffer, clientIPs[fds[i].fd]);

                            else if (sBuffer.substr(0, 4) == "MODE")
                                Mode(i, sBuffer, fds);

                            else if (sBuffer.substr(0, 5) == "TOPIC")
                                Topic(i, sBuffer, fds);

                            else if (sBuffer.substr(0, 6) == "INVITE")
                                Invite(i, sBuffer, fds);

                            else if (sBuffer.substr(0, 4) == "KICK")
                                Kick(i, sBuffer, fds);

                            else if (sBuffer.substr(0, 4) == "PART")
                                part(fds[i].fd, sBuffer);

                            else if (sBuffer.substr(0, 7) == "PRIVMSG")
                                privmsg(sBuffer, fds[i].fd, clientIPs[fds[i].fd]);

                            else if (sBuffer.substr(0, 4) != "PONG")
                                pong(sBuffer, fds[i].fd);
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
        if (users[i].getSocket() == user.getSocket())
        {
            users.erase(users.begin() + i);
            return;
        }
    }
}

void server::addChannel(std::string ChannelName, user user, std::string key)
{
    (void)key;
    bool channelExists = false;
    if ((ChannelName[0] != '#' && ChannelName[0] != '&') || ChannelName.length() > 64)
    {
        std::string reply = ERR_BADCHANNELNAME(user.getNick(), user.getIpAddress(), ChannelName);
        send(user.getSocket(), reply.c_str(), reply.size(), 0);
        return;
    }
    int channelsSize = channels.size();
    for (int i = 0; i < channelsSize; i++)
    {
        if (channels[i].getName() == ChannelName)
        {
            if (channels[i].getInviteOnly() && !channels[i].isInvited(user))
            {
                std::string reply = ERR_INVITEONLY(user.getNick(), ChannelName);
                send(user.getSocket(), reply.c_str(), reply.size(), 0);
                return;
            }
            if (channels[i].getHaskey() && channels[i].getKey() != key)
            {
                std::string reply = ERR_BADCHANNELKEY(user.getNick(), serverIP, ChannelName);
                send(user.getSocket(), reply.c_str(), reply.size(), 0);
                return;
            }
            if (channels[i].getHasLimit() && channels[i].getMembers().size() == channels[i].getLimit())
            {
                std::string reply = ERR_CHANNELISFULL(user.getNick(), ChannelName);
                send(user.getSocket(), reply.c_str(), reply.size(), 0);
                return;
            }

            channels[i].addMember(user);
            if (channels[i].getInviteOnly())
                channels[i].eraseInvited(user);
            for (size_t j = 0; j < channels[i].getMembers().size(); j++)
            {
                std::string reply = RPL_JOIN(user.getNick(), user.getUserName(), ChannelName, user.getIpAddress());
                send(channels[i].getMembers()[j].getSocket(), reply.c_str(), reply.size(), 0);
            }

            std::string users;
            for (size_t j = 0; j < channels[i].getMembers().size(); j++)
            {
                if (channels[i].isoperator(channels[i].getMembers()[j]))
                    users += "@" + channels[i].getMembers()[j].getNick() + " ";
                else
                    users += channels[i].getMembers()[j].getNick() + " ";
            }
            std::string reply = RPL_NAMREPLY(serverIP, users, ChannelName, user.getNick());
            send(user.getSocket(), reply.c_str(), reply.size(), 0);
            // TOPIC reply
            if (channels[i].getHasTopic())
            {
                reply = RPL_TOPICDISPLAY(serverIP, user.getNick(), ChannelName, channels[i].getTopic());
                send(user.getSocket(), reply.c_str(), reply.size(), 0);
            }
            channelExists = true;
            break;
        }
    }
    if (channelExists)
    {
        return;
    }
    class channel newChannel(ChannelName);
    newChannel.addMember(user);
    newChannel.addOperator(user);
    channels.push_back(newChannel);
    std::string reply = RPL_JOIN(user.getNick(), user.getUserName(), ChannelName, user.getIpAddress());
    send(user.getSocket(), reply.c_str(), reply.size(), 0);
    reply = RPL_NAMREPLY(serverIP, "@" + user.getNick() + " ", ChannelName, user.getNick());
    send(user.getSocket(), reply.c_str(), reply.size(), 0);
}

int server::getUserBySocket(int socket)
{
    size_t usersSize = users.size();
    for (size_t i = 0; i < usersSize; i++)
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
    std::vector<user> users = getUsers();
    std::string reply;
    bool userExists = false;
    for (size_t j = 0; j < users.size(); j++)
    {
        if (users[j].getNick() == receiver && users[j].getRegistered() && users[j].getNick() != sender.getNick())
        {
            userExists = true;
            reply = PRIVMSG_FORMAT(sender.getNick(), sender.getUserName(), sender.getIpAddress(), receiver, message);
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
    for (size_t j = 0; j < channels.size(); j++)
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
            for (size_t k = 0; k < members.size(); k++)
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
        reply = ERR_NOSUCHCHANNEL(serverIP, receiverchannel, sender.getNick());
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

void server::pass(int fd, std::string sBuffer, std::string clientIP)
{
    try
    {

        std::vector<std::string> args = splitCommand(sBuffer);
        int user = getUserBySocket(fd);
        if (args.size() < 2)
        {
            std::string reply = ERR_NEEDMOREPARAMS(clientIP, serverIP, "PASS");
            send(fd, reply.c_str(), reply.size(), 0);
            return;
        }
        if (users[user].getPasswordCorrect())
        {
            std::string reply = ERR_ALREADYREGISTERED(clientIP, serverIP);
            send(fd, reply.c_str(), reply.size(), 0);
            return;
        }
        if (args[1] != password)
        {
            std::string reply = ERR_PASSWDMISMATCH(clientIP, serverIP);
            send(fd, reply.c_str(), reply.size(), 0);
        }
        else
        {
            if (user != -1)
                users[user].setPassConfirmed(true);
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

void server::nick(int fd, std::string sBuffer, std::string clientIP)
{
    std::vector<std::string> args = splitCommand(sBuffer);
    if (args.size() < 2)
    {
        std::string reply = ERR_NONICKNAMEGIVEN(clientIP, serverIP);
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }
    try
    {
        if (args[1][0] == ':')
            args[1] = args[1].substr(1, args[1].size() - 1);
        if (args[1].size() > 9 || (isalpha(args[1][0]) == 0 && args[1][0] != '_' && args[1][0] != '[' && args[1][0] != ']' && args[1][0] != '\\' && args[1][0] != '`' && args[1][0] != '^' && args[1][0] != '{' && args[1][0] != '}' && args[1][0] != '|' && args[1][0] != '-'))
        {
            std::string reply = ERR_ERRONEUSNICKNAME(clientIP, serverIP);
            send(fd, reply.c_str(), reply.size(), 0);
            return;
        }
        for (size_t i = 1; i < args[1].size(); i++)
        {
            if (isalnum(args[1][i]) == 0 && args[1][i] != '_' && args[1][i] != '[' && args[1][i] != ']' && args[1][i] != '\\' && args[1][i] != '`' && args[1][i] != '^' && args[1][i] != '{' && args[1][i] != '}' && args[1][i] != '|' && args[1][i] != '-')
            {
                std::string reply = ERR_ERRONEUSNICKNAME(clientIP, serverIP);
                send(fd, reply.c_str(), reply.size(), 0);
                return;
            }
        }
        if (args[1].size() == 0)
        {
            std::string reply = ERR_NONICKNAMEGIVEN(clientIP, serverIP);
            send(fd, reply.c_str(), reply.size(), 0);
            return;
        }
        int user = getUserBySocket(fd);
        if (user != -1)
            users[user].setNick(args[1], users, serverIP);
    }
    catch (std::exception &e)
    {
        std::cerr << "Error NICK: " << e.what() << "\n";
    }
}

void server::usercmd(int fd, std::string sBuffer, std::string clientIP)
{
    std::vector<std::string> args = splitCommand(sBuffer);
    if (args.size() < 5)
    {
        std::string reply = ERR_NEEDMOREPARAMS(clientIP, serverIP, "USER");
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }
    try
    {
        int user = getUserBySocket(fd);
        for (size_t i = 0; i < args[1].size(); i++)
        {
            if (args[1][i] == ' ')
            {
                args[1] = args[1].substr(0, i);
                break;
            }
        }
        if (user != -1)
        {
            users[user].setUserName(args[1], serverIP);
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "Error USER: " << e.what() << "\n";
    }
}

void server::join(int fd, std::string sBuffer, std::string clientIP)
{
    try
    {
        int joinedUser = getUserBySocket(fd);
        std::vector<std::string> args = splitCommand(sBuffer);
        if (args.size() < 2)
        {
            std::string reply = ERR_NEEDMOREPARAMS(clientIP, serverIP, "JOIN");
            send(fd, reply.c_str(), reply.size(), 0);
            return;
        }
        std::string channelsName = args[1];
        std::string keys = args[2];
        std::map<std::string, std::string> chans = parseChannels(channelsName, keys);
        std::map<std::string, std::string>::iterator it = chans.begin();
        while (it != chans.end())
        {
            addChannel(it->first, users[joinedUser], it->second);
            it++;
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "Error JOIN: " << e.what() << "\n";
    }
}

void server::quit(int fd, size_t &i)
{
    try
    {
        int user = getUserBySocket(fd);
        close(fd);
        clientIPs.erase(fd);
        fds.erase(fds.begin() + i);
        --i;
        if (user != -1)
        {
            for (size_t j = 0; j < channels.size(); j++)
            {
                if (channels[j].isMember(users[user]))
                {
                    channels[j].removeMember(users[user], 1);
                    if (channels[j].getMembers().size() == 0)
                    {
                        removeChannel(channels[j]);
                        j--;
                    }
                }
            }
            removeUser(users[user]);
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

void server::part(int fd, std::string sBuffer)
{
    try
    {
        int partUser = getUserBySocket(fd);
        if (partUser == -1)
            throw serverException("User not found");
        std::string channelName = sBuffer.substr(5, sBuffer.size() - 1);
        for (size_t i = 0; i < channelName.size(); i++)
        {
            if (channelName[i] == ' ' || channelName[i] == ',')
            {
                channelName = channelName.substr(0, i);
                break;
            }
        }
        for (size_t i = 0; i < channels.size(); i++)
        {
            if (channels[i].getName() == channelName)
            {
                channels[i].removeMember(users[partUser], 1);
                if (channels[i].getMembers().size() == 0)
                {
                    removeChannel(channels[i]);
                    i--;
                }
            }
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "Error PART: " << e.what() << "\n";
    }
}

void server::privmsg(std::string sBuffer, int fd, std::string clientIP)
{
    std::vector<std::string> args = splitCommand(sBuffer);
    if (args.size() < 3)
    {
        std::string reply = ERR_NEEDMOREPARAMS(clientIP, serverIP, "PRIVMSG");
        send(fd, reply.c_str(), reply.size(), 0);
        return;
    }
    std::string receiver = args[1];
    std::string message;
    message = sBuffer.substr(sBuffer.find(args[2]), sBuffer.size() - sBuffer.find(args[2]));
    std::vector<std::string> receivers;
    for (size_t i = 0; i < receiver.size(); i++)
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
    try
    {
        int sender = getUserBySocket(fd);
        if (sender == -1)
            throw serverException("User not found");
        for (size_t i = 0; i < receivers.size(); i++)
        {
            if (receivers[i][0] == '#' || receivers[i][0] == '&')
                prvmsgchannel(users[sender], receivers[i], message);
            else
                prvmsg(users[sender], receivers[i], message);
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "Error PRIVMSG: " << e.what() << "\n";
    }
}

void server::pong(std::string sBuffer, int fd)
{
    std::string command = sBuffer.substr(0, sBuffer.size() - 1);
    if (command[command.size() - 1] == '\n')
        command = command.substr(0, command.size() - 2);
    else
        command = command.substr(0, command.size() - 3);
    for (size_t i = 0; i < command.size(); i++)
    {
        if (command[i] == ' ')
        {
            command = command.substr(0, i);
            break;
        }
    }
    int user = getUserBySocket(fd);
    if (user != -1)
    {
        std::string reply = ERR_UNKNOWNCOMMAND(users[user].getNick(), serverIP, command);
        send(fd, reply.c_str(), reply.size(), 0);
    }
}

void server::newConnections()
{
    try
    {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(listeningSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSocket < 0)
        {
            throw serverException("Error accepting connection\n");
        }
        if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) < 0)
        {
            throw serverException("Error setting socket to non-blocking\n");
        }

        clientIPs[clientSocket] = inet_ntoa(clientAddr.sin_addr);

        struct pollfd clientFd;
        clientFd.fd = clientSocket;
        clientFd.events = POLLIN;
        fds.push_back(clientFd);
        user newUser(clientIPs[clientSocket], clientSocket);
        users.push_back(newUser);
    }
    catch (std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

void server::disconnecting(size_t &i)
{
    std::cout << "client :" << clientIPs[fds[i].fd] << " with socket: " << fds[i].fd << " has disconnected\n";
    int fd = fds[i].fd;
    close(fd);
    clientIPs.erase(fd);
    fds.erase(fds.begin() + i);
    --i;
    try
    {
        if (getUserBySocket(fd) != -1)
        {
            std::cerr << "User: " << users[getUserBySocket(fd)].getNick() << " has disconnected\n";
            for (size_t j = 0; j < channels.size(); j++)
            {
                if (channels[j].isMember(users[getUserBySocket(fd)]))
                {
                    channels[j].removeMember(users[getUserBySocket(fd)], 1);
                    if (channels[j].getMembers().size() == 0)
                    {
                        removeChannel(channels[j]);
                        j--;
                    }
                }
            }
            removeUser(users[getUserBySocket(fd)]);
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
    }
}