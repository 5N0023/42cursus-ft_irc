#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <exception>
#include "user.hpp"
#include "channel.hpp"
#include "replies.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <poll.h>
#include <map>
#include <fcntl.h>
#include "utils.hpp"
#include <sstream>
class user;
class channel;
class server
{
private:
    int port;
    std::string password;
    std::vector<user> users;
    std::vector<channel> channels;
    std::string serverIP;
    int listeningSocket;
    std::map<int, std::string> clientIPs;
    std::vector<struct pollfd> fds;
    struct pollfd listenFd;

public:
    server(int port, std::string password);
    ~server();
    void run();
    void stop();

    void newConnections();
    void disconnecting(size_t &i);
    void addUser(user newUser);
    void removeUser(user user);
    void Invite(size_t, std::string, std::vector<struct pollfd>);
    void Mode(size_t, std::string, std::vector<struct pollfd>);
    void Topic(size_t, std::string, std::vector<struct pollfd>);
    void Kick(size_t, std::string, std::vector<struct pollfd>);
    void pass(int socket, std::string sBuffer, std::string clientIP);
    void nick(int socket, std::string sBuffer, std::string clientIP);
    void usercmd(int fd, std::string sBuffer, std::string clientIP);
    void join(int fd, std::string sBuffer, std::string clientIP);
    void quit(int fd, size_t &i);
    void addChannel(std::string ChannelName, user user, std::string key);
    void removeChannel(channel channel);
    void prvmsg(user sender, std::string receiver, std::string message);
    void prvmsgchannel(user sender, std::string channel, std::string message);
    void part(int fd, std::string sBuffer);
    void privmsg(std::string sBuffer,int fd, std::string clientIP);
    void pong(std::string sBuffer,int fd);
    std::vector<user> &getUsers();
    std::vector<channel> &getChannels();
    int getUserBySocket(int socket);
    class serverException : public std::exception
    {
    private:
        std::string message;

    public:
        serverException(std::string message);
        const char *what(void) const throw();
        ~serverException() throw();
    };
};