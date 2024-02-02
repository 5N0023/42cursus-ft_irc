#include "server.hpp"

server::server(int port, std::string password): port(port), password(password)
{
    listeningSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listeningSocket < 0)
    {
        throw serverException("Error creating socket");
    }
    int opt = 1;
    if (setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        throw serverException("Error setting socket options");
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(listeningSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        throw serverException("Error binding socket");
    }

    // Listen on socket
    if (listen(listeningSocket, 1024) < 0)
    {
        throw serverException("Error listening on socket");
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

    // Map to store client IP addresses
    std::unordered_map<int, std::string> clientIPs;

    while (true)
    {
        int ret = poll(fds.data(), fds.size(), -1); // -1 means wait indefinitely
        if (ret < 0)
        {
            throw serverException("Error on poll()");
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
                        throw serverException("Error accepting connection");
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
                    }
                    else
                    {
                        // Process received data
                        std::cout << "Received from " << clientIPs[fds[i].fd] << ": " << std::string(buffer, bytesRead) << "\n";
                        if (memcmp(buffer, "NICK", 4) == 0)
                        {
                            std::string nick = std::string(buffer + 5, bytesRead - 5);
                            try {
                            this->addUser(user("test", nick, clientIPs[fds[i].fd], fds[i].fd, this->getUsers()));
                            }
                            catch (user::userException &e)
                            {
                                std::cerr << "Error: " << e.what() << "\n";
                            }
                        }
                        if (memcmp(buffer, "JOIN", 4) == 0)
                        {
                            channel newChannel(std::string(buffer + 5, bytesRead - 5));
                            user joinedUser = this->getUserBySocket(fds[i].fd);
                            std::string channel = std::string(buffer + 5, bytesRead - 5);
                            try {
                                this->addChannel(newChannel, joinedUser);
                            }
                            catch (channel::channelException &e)
                            {
                                std::cerr << "Error: " << e.what() << "\n";
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
        if (users[i].getUserName() == newUser.getUserName())
        {
            throw serverException("Username already in use");
        }
    }
    users.push_back(newUser);
    std::string reply = RPL_WELCOME(newUser.getNick(), newUser.getIpAddress());
    send(newUser.getSocket(), reply.c_str(), reply.size(), 0);
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
    throw serverException("User not found");
}


void server::addChannel(channel newChannel, user user)
{
    int channelsSize = channels.size();
    for (int i = 0; i < channelsSize; i++)
    {
        if (channels[i].getName() == newChannel.getName())
        {
            std::string reply = RPL_JOIN(user.getNick(), user.getUserName(), newChannel.getName(), user.getIpAddress());
            send(user.getSocket(), reply.c_str(), reply.size(), 0);
            return;
        }
    }
    channels.push_back(newChannel);
}

user server::getUserBySocket(int socket)
{
    int usersSize = users.size();
    for (int i = 0; i < usersSize; i++)
    {
        if (users[i].getSocket() == socket)
        {
            return users[i];
        }
    }
    throw serverException("User not found");
}


// int main() {
//     int listeningSocket = socket(AF_INET, SOCK_STREAM, 0);
//     if (listeningSocket < 0) {
//         std::cerr << "Error creating socket\n";
//         return -1;
//     }

//     struct sockaddr_in serverAddr;
//     memset(&serverAddr, 0, sizeof(serverAddr));
//     serverAddr.sin_family = AF_INET;
//     serverAddr.sin_port = htons(PORT);
//     serverAddr.sin_addr.s_addr = INADDR_ANY;

//     if (bind(listeningSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
//         std::cerr << "Error binding socket\n";
//         close(listeningSocket);
//         return -1;
//     }

//     if (listen(listeningSocket, 5) < 0) {
//         std::cerr << "Error listening on socket\n";
//         close(listeningSocket);
//         return -1;
//     }

//     std::vector<struct pollfd> fds;
//     struct pollfd listenFd;
//     listenFd.fd = listeningSocket;
//     listenFd.events = POLLIN;
//     fds.push_back(listenFd);

//     // Map to store client IP addresses
//     std::unordered_map<int, std::string> clientIPs;

//     while (true) {
//         int ret = poll(fds.data(), fds.size(), -1); // -1 means wait indefinitely
//         if (ret < 0) {
//             std::cerr << "Error on poll()\n";
//             break;
//         }

//         for (size_t i = 0; i < fds.size(); ++i) {
//             if (fds[i].revents & POLLIN) {
//                 if (fds[i].fd == listeningSocket) {
//                     // Accept new connection
//                     struct sockaddr_in clientAddr;
//                     socklen_t clientAddrLen = sizeof(clientAddr);
//                     int clientSocket = accept(listeningSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
//                     if (clientSocket < 0) {
//                         std::cerr << "Error accepting connection\n";
//                         continue;
//                     }

//                     // Store client IP address
//                     clientIPs[clientSocket] = inet_ntoa(clientAddr.sin_addr);

//                     struct pollfd clientFd;
//                     clientFd.fd = clientSocket;
//                     clientFd.events = POLLIN;
//                     fds.push_back(clientFd);
//                 } else {
//                     // Handle data from client
//                     char buffer[1024];
//                     int bytesRead = recv(fds[i].fd, buffer, sizeof(buffer), 0);
//                     if (bytesRead <= 0) {
//                         // Connection closed or error
//                         close(fds[i].fd);
//                         clientIPs.erase(fds[i].fd); // Remove IP from map
//                         fds.erase(fds.begin() + i);
//                         --i; // Adjust index after erase
//                     } else {
//                         // Process received data
//                         std::cout << "Received from " << clientIPs[fds[i].fd] << ": " << std::string(buffer, bytesRead) << "\n";
//                         if (memcmp(buffer, "JOIN", 4) == 0) {
//                             // Send welcome message
//                             std::string channel = std::string(buffer + 5, bytesRead - 5);
//                             //":" + nick + "!~" + username + "@" + ipaddress + " JOIN " + channelname + "\r\n"
//                             std::string nick = "test";
//                             std::string reply = ":" + nick + "!~" + "test" + "@" + clientIPs[fds[i].fd] + " JOIN " + channel + "\r\n";
//                             send(fds[i].fd, reply.c_str(), reply.size(), 0);
//                         }
//                     }
//                 }
//             }
//         }
//     }

//     for (auto& fd : fds) {
//         close(fd.fd);
//     }
//     close(listeningSocket);
//     return 0;
// }