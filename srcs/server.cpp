/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hznagui <hznagui@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 11:08:11 by hznagui           #+#    #+#             */
/*   Updated: 2024/03/09 21:54:14 by hznagui          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

server::server(int Port, std::string password): password(password)
{
    port = Port;
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
    serverIP = getLocalIP();
    std::cerr << "serverIP: " << serverIP << std::endl;


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
                                for (size_t j = 0; j < channels.size(); j++)
                                {
                                    if (channels[j].isMember(users[this->getUserBySocket(fds[i].fd)]))
                                    {
                                        channels[j].removeMember(users[this->getUserBySocket(fds[i].fd)],1);
                                        if (channels[j].getMembers().size() == 0)
                                        {
                                            this->removeChannel(channels[j]);
                                            j--;
                                        }
                                    }
                                }
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
                                }
                            catch (user::userException &e)
                                {
                                    std::cerr << "Error: " << e.what() << "\n";
                                }
                        }
                        std::string sBuffer = std::string(buffer, bytesRead);
                        // std::cerr << "asdsd :: " << sBuffer << ":::" << std::endl;
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
                        // trim the buffer
                        if (sBuffer.find("\r\n")!=  std::string::npos)
                            sBuffer = sBuffer.substr(0, sBuffer.size() -2);
                        if (sBuffer.find("\n")!=  std::string::npos)
                            sBuffer = sBuffer.substr(0, sBuffer.size() -1);
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
                                std::cerr << "args[1] which is password : |" << args[1] << "|" << std::endl;
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
                                {
                                    for (size_t j = 0; j < channels.size(); j++)
                                    {
                                        if (channels[j].isMember(users[user]))
                                        {
                                            channels[j].removeMember(users[user],1);
                                            if (channels[j].getMembers().size() == 0)
                                            {
                                                this->removeChannel(channels[j]);
                                                j--;
                                            }
                                        }
                                    }
                                    this->removeUser(users[user]);
                                }
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
                                    users[user].setNick(args[1], users,serverIP);
                            }
                            catch (user::userException &e)
                            {
                                std::cerr << "Error NICK: " << e.what() << "\n";
                            }
                        }
                        else if (sBuffer.substr(0, 4) == "USER")
                        {
                            std::cerr << "USER command : " << sBuffer << std::endl;
                            std::vector<std::string> args = splitCommand(sBuffer);
                            if (args.size() < 5)
                            {
                                std::string reply = ERR_NEEDMOREPARAMS(clientIPs[fds[i].fd], serverIP,"USER");
                                send(fds[i].fd, reply.c_str(), reply.size(), 0);
                                continue;
                            }
                            try {
                                int user = this->getUserBySocket(fds[i].fd);
                                for (size_t i = 0; i < args[1].size(); i++)
                                {
                                    if (args[1][i] == ' ')
                                    {
                                        args[1] = args[1].substr(0, i);
                                        break;
                                    }
                                }
                                if (user != -1)
                                    users[user].setUserName(args[1],serverIP);
                            }
                            catch (user::userException &e)
                            {
                                std::cerr << "Error USER: " << e.what() << "\n";
                            }
                        }
                        else if (sBuffer.substr(0, 4) == "JOIN")
                        {//add condition of checking if the channel is invite only && check if it have password or not 
                            try {
                                int joinedUser = this->getUserBySocket(fds[i].fd);
                                std::vector<std::string> args = splitCommand(sBuffer);
                                if (args.size() < 2)
                                {
                                    std::string reply = ERR_NEEDMOREPARAMS(clientIPs[fds[i].fd], serverIP, "JOIN");
                                    send(fds[i].fd, reply.c_str(), reply.size(), 0);
                                    continue;
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
                            catch (channel::channelException &e)
                            {
                                std::cerr << "Error JOIN: " << e.what() << "\n";
                            }
                        }
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        //my starting
                        else if (sBuffer.substr(0,4)=="MODE")
                        {
                            int User = this->getUserBySocket(fds[i].fd);
                            try {
                                
                                std::vector<std::string> vec = split(sBuffer,' ');
                                // for (size_t f=0;f<vec.size();f++)   
                                //     std::cerr<<"'"<<vec[f]<<"'";
                                //     std::cerr<<vec.size();
                                // std::cerr<<std::endl;
                                if (vec.size() < 3)
                                   throw (channel::channelException(ERR_NEEDMOREPARAMS(users[User].getNick() ,serverIP,"MODE")));
                                size_t it = 0;
                                    for ( ;it < channels.size();it++)
                                    {
                                        if (channels[it].getName() == vec[1])
                                        {
                                            if (channels[it].isMember(users[User]))
                                            {
                                                if (channels[it].isoperator(users[User]))  
                                                {
                                                    // std::cerr << "ha howa dkhal"<<std::endl;
                                                    if (vec[2][0] != '+' && vec[2][0] != '-')
                                                        throw channel::channelException(ERR_UNKNOWNMODE(users[User].getNick(),serverIP,channels[it].getName(),vec[0][0]));
                                                    bool positive = (vec[2][0] == '-' ? false : true);
                                                    std::string ret;
                                                    size_t arg=3;
                                                    ret+= vec[2][0];
                                                    for (size_t k = 1; k < vec[2].size(); k++)
                                                    {
                                                        try
                                                        {
                                                            if (vec[2][k] == 'i' )
                                                            {
                                                                if (positive != channels[it].getMode())
                                                                {
                                                                        channels[it].setMode(positive);
                                                                        ret += 'i';
                                                                }
                                                            }
                                                            else if (vec[2][k] == 't' )
                                                            {
                                                                if (positive != channels[it].getTopicStrict())
                                                                {
                                                                        channels[it].setTopicStrict(positive);
                                                                        ret+='t';
                                                                }
                                                            }
                                                            else if (vec[2][k] == 'k')
                                                            {
                                                                if (!positive && channels[it].getHaskey())
                                                                {
                                                                    channels[it].setKey("");
                                                                    ret += 'k';
                                                                    channels[it].setHaskey(positive);
                                                                }
                                                                else if (vec.size() > arg  && (positive && !channels[it].getHaskey()))
                                                                {
                                                                    channels[it].setKey(vec[arg]);
                                                                    ret += 'k';
                                                                    arg++;
                                                                    channels[it].setHaskey(positive);
                                                                }
                                                            }
                                                            else if (vec[2][k] == 'o' )
                                                            {
                                                                if (vec.size() > arg)
                                                                {
                                                                    user tmp = getUser_str(vec[arg], users);
                                                                    if (tmp.getSocket() == -1 && tmp.getIpAddress() == "error")
                                                                        throw channel::channelException(ERR_NOSUCHNICK(serverIP,vec[arg]));
                                                                    else if (!channels[it].isMember(tmp) || (channels[it].isoperator(tmp) && positive))
                                                                    {}
                                                                    else
                                                                    {
                                                                        channels[it].AddEraseOperator(tmp);
                                                                        ret += 'o';
                                                                        arg++;
                                                                    }
                                                                }
                                                            }
                                                            else if (vec[2][k] == 'l')
                                                            {
                                                                if (positive && vec.size() > arg && has_char(vec[arg]) && (channels[it].getLimit()  != ToSize_T(vec[arg])||!channels[it].getHasLimit()))
                                                                {
                                                                    channels[it].setHasLimit(positive);
                                                                    channels[it].setLimit(ToSize_T(vec[arg]));
                                                                    ret += 'l';
                                                                    arg++;
                                                                }
                                                                else if (!positive && channels[it].getHasLimit())
                                                                {
                                                                    channels[it].setHasLimit(positive);
                                                                    ret += 'l';
                                                                }
                                                            }
                                                            else
                                                                throw channel::channelException(ERR_UNKNOWNMODE(users[User].getNick(),serverIP,channels[it].getName(),vec[2][k]));

                                                        }
                                                        catch (channel::channelException &e)
                                                        {
                                                            std::string replay = e.what();
                                                            send(fds[i].fd, replay.c_str(), replay.size(), 0);
                                                            if (replay == ERR_NOSUCHNICK(serverIP,vec[arg]))
                                                                break;
                                                        }
                                                    }
                                                    if (ret.size() > 1)
                                                    {
                                                        if (arg != 3)
                                                        {
                                                            for (size_t hh = 3 ;arg!=hh;hh++)
                                                                ret += + " " + vec[hh];
                                                        }
                                                        std::vector<user> tmpusers = channels[it].getMembers();
                                                        std::string reply = RPL_CHANNELMODEIS(users[User].getNick(),serverIP,channels[it].getName(),ret);
                                                        for (size_t s = 0 ; s < tmpusers.size() ; s++)
                                                            send(tmpusers[s].getSocket(), reply.c_str(), reply.size(), 0);
                                                    }
                                                    
                                                }
                                                else
                                                    throw channel::channelException(ERR_CHANOPRIVSNEEDED(serverIP,channels[it].getName()));
                                            }
                                            else
                                                throw channel::channelException(ERR_NOTONCHANNEL(serverIP,channels[it].getName()));
                                            break;
                                        }
                                    }
                                        if (it == channels.size())
                                            throw channel::channelException(ERR_NOSUCHCHANNEL(serverIP, vec[1],users[User].getNick()));
                                }
                            catch (channel::channelException &e)
                            {
                                std::string replay= e.what();
                                send(fds[i].fd, replay.c_str(), replay.size(), 0);
                            }
                            
                        }
                        else if (sBuffer.substr(0,5)=="TOPIC")
                        {
                            int User = this->getUserBySocket(fds[i].fd);
                            try {
                                std::vector<std::string> vec = split(sBuffer,' ');
                                if (vec.size() < 2)
                                   throw (channel::channelException(ERR_NEEDMOREPARAMS(users[User].getNick() ,serverIP,"TOPIC")));
                                if (vec.size() != 2 && vec[2][0] == ':') 
                                    vec[2]=vec[2].substr(1);
                                size_t it = 0;
                                    for ( ;it < channels.size();it++)
                                    {
                                        if (channels[it].getName() == vec[1])
                                        {
                                            if (channels[it].isMember(users[User]))
                                            {
                                                if (vec.size() == 2)
                                                    {
                                                        if (channels[it].getHasTopic() == false)
                                                            throw channel::channelException(RPL_NOTOPIC(serverIP,channels[it].getName(),users[User].getNick()));
                                                        std::string replay = RPL_TOPICDISPLAY(serverIP,users[User].getNick(),channels[it].getName(),channels[it].getTopic());
                                                        send(fds[i].fd, replay.c_str(), replay.size(), 0);
                                                    }
                                                else if (channels[it].isoperator(users[User]) || !channels[it].getTopicStrict())  
                                                {
                                                    std::string tmp;
                                                    for (size_t h=2;h < vec.size();h++)
                                                            tmp +=" "+ vec[h];
                                                    channels[it].setTopic(tmp);
                                                    channels[it].setHasTopic(true);
                                                    std::vector<user> tmpusers = channels[it].getMembers();
                                                    std::string reply = RPL_SETTOPIC(users[User].getNick(),serverIP,channels[it].getName(),tmp);
                                                    for (size_t s = 0 ; s < tmpusers.size() ; s++)
                                                        send(tmpusers[s].getSocket(), reply.c_str(), reply.size(), 0);
                                                }
                                                else
                                                    throw channel::channelException(ERR_CHANOPRIVSNEEDED(serverIP,channels[it].getName()));
                                            }
                                            else
                                                throw channel::channelException(ERR_NOTONCHANNEL(serverIP,channels[it].getName()));
                                            break;
                                        }
                                    }
                                        if (it == channels.size())
                                            throw channel::channelException(ERR_NOSUCHCHANNEL(serverIP, vec[1],users[User].getNick()));//khesni ne3raf channel li jani mena msg
                                }
                            catch (channel::channelException &e)
                            {
                                std::string replay= e.what();
                                send(fds[i].fd, replay.c_str(), replay.size(), 0);
                            }
                        }
                        else if (sBuffer.substr(0,6) == "INVITE")
                        {
                            int User = this->getUserBySocket(fds[i].fd);
                            try {
                                std::vector<std::string> vec = split(sBuffer,' ');
                                if (vec.size() < 3)
                                   throw (channel::channelException(ERR_NEEDMOREPARAMS(users[User].getNick() ,serverIP,"INVITE")));
                                if(vec[2][0]==':') 
                                    vec[2]=vec[2].substr(1);
                                size_t it = 0;
                                    for ( ;it < channels.size();it++)
                                    {
                                        if (channels[it].getName() == vec[2])
                                        {
                                            if (channels[it].isMember(users[User]))
                                            {
                                                if (channels[it].isoperator(users[User]))  
                                                {
                                                    user tmp1 = getUser_str(vec[1], users);
                                                    if (tmp1.getSocket() == -1 && tmp1.getIpAddress() == "error")
                                                        throw channel::channelException(ERR_NOSUCHNICK(serverIP,vec[1]));
                                                    else
                                                    {
                                                        user tmp = getUser_str(vec[1], channels[it].getMembers());
                                                        if (tmp.getSocket() == -1 && tmp.getIpAddress() == "error")
                                                        {
                                                            std::string reply = RPL_INVITE(users[User].getNick(),users[User].getNick(),serverIP,vec[1],vec[2]);
                                                            channels[it].addInvite(getUser_str(vec[1],users));
                                                            send(tmp1.getSocket(), reply.c_str(), reply.size(), 0);
                                                        }
                                                        else
                                                            throw channel::channelException(ERR_USERONCHANNEL(serverIP,vec[2],vec[1]));
                                                    }
                                                }
                                                else
                                                    throw channel::channelException(ERR_CHANOPRIVSNEEDED(serverIP,channels[it].getName()));
                                            }
                                            else
                                                throw channel::channelException(ERR_NOTONCHANNEL(serverIP,channels[it].getName()));
                                            break;
                                        }
                                    }
                                        if (it == channels.size())
                                            throw channel::channelException(ERR_NOSUCHCHANNEL(serverIP, vec[2],users[User].getNick()));//khesni ne3raf channel li jani mena msg
                                }
                    
                                
                            catch (channel::channelException &e)
                            {
                                std::string replay= e.what();
                                send(fds[i].fd, replay.c_str(), replay.size(), 0);
                            }
                        }
                        
                        else if (sBuffer.substr(0,4) == "KICK")
                        {
                            int User = this->getUserBySocket(fds[i].fd);
                            try {
                                std::vector<std::string> vec = split(sBuffer,' ');
                                if (vec.size() < 4)
                                   throw (channel::channelException(ERR_NEEDMOREPARAMS(users[User].getNick() ,serverIP,"KICK")));
                                std::vector<std::string> chan = split(vec[1],',') , target = split(vec[2],',');
                                for (size_t y = 0; y < chan.size();y++)
                                {
                                size_t it = 0;
                                    try
                                    {
                                        for ( ;it < channels.size();it++)
                                        {
                                            if (channels[it].getName() == chan[y])
                                            {
                                                if (channels[it].isMember(users[User]))
                                                {
                                                    if (channels[it].isoperator(users[User]))  
                                                    {
                                                        for (size_t x = 0;x<target.size();x++)
                                                        {
                                                            try{ 
                                                                user tmp = getUser_str(target[x],channels[it].getMembers());
                                                                if (tmp.getSocket() == -1 && tmp.getIpAddress() == "error")
                                                                        throw channel::channelException(ERR_NOSUCHNICK(serverIP,target[x]));
                                                                else 
                                                                {
                                                                    std::string reply = RPL_KICK(users[User].getNick(),users[User].getNick(),serverIP,chan[y],target[x],getreason(vec,3));
                                                                    std::vector<user> tmpusers = channels[it].getMembers();
                                                                    for (size_t s = 0 ; s < tmpusers.size() ; s++)
                                                                        send(tmpusers[s].getSocket(), reply.c_str(), reply.size(), 0);
                                                                    channels[it].removeMember(tmp,0);
                                                                }
                                                            }
                                                            catch (channel::channelException &e)
                                                            {
                                                                std::string replay= e.what();
                                                                send(fds[i].fd, replay.c_str(), replay.size(), 0);
                                                            }
                                                            it=channels.size()+1;
                                                        }
                                                    }
                                                    else
                                                        throw channel::channelException(ERR_CHANOPRIVSNEEDED(serverIP,channels[it].getName()));
                                                }
                                                else
                                                    throw channel::channelException(ERR_NOTONCHANNEL(serverIP,channels[it].getName()));
                                                break;
                                            }
                                        }
                                        if (it == channels.size())
                                            throw channel::channelException(ERR_NOSUCHCHANNEL(serverIP, chan[y],users[User].getNick()));//khesni ne3raf channel li jani mena msg
                                    }
                                    catch (channel::channelException &e)
                                    {
                                        std::string replay= e.what();
                                        send(fds[i].fd, replay.c_str(), replay.size(), 0);
                                    }
                                }
                                }
                            catch (channel::channelException &e)
                           {
                                std::string replay= e.what();
                                send(fds[i].fd, replay.c_str(), replay.size(), 0);
                            }
                        }    
                        //my ending 
                        else if (sBuffer.substr(0, 4) == "PART")
                        {
                            try {
                                int partUser = this->getUserBySocket(fds[i].fd);
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
                                        this->channels[i].removeMember(users[partUser],1);
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
                            std::vector<std::string> args = splitCommand(sBuffer);
                            if (args.size() < 3)
                                {
                                    std::string reply = ERR_NEEDMOREPARAMS(clientIPs[fds[i].fd], serverIP, "PRIVMSG");
                                    send(fds[i].fd, reply.c_str(), reply.size(), 0);
                                    continue;
                                }
                            std::string receiver = args[1];
                            std::string message;
                            message = sBuffer.substr(sBuffer.find(args[2]), sBuffer.size() - sBuffer.find(args[2]));
                            std::cerr << "message: " << message << "|" << std::endl;
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
                            try {
                                int sender = this->getUserBySocket(fds[i].fd);
                                if (sender == -1)
                                    throw serverException("User not found");
                                for (size_t i = 0; i < receivers.size(); i++)
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
                            for (size_t i = 0; i < command.size(); i++)
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
        if (users[i].getSocket() == user.getSocket())
        {
            users.erase(users.begin() + i);
            return;
        }
    }
    throw serverException("User not found in server\n");
}


void server::addChannel(std::string ChannelName, user user,std::string key)
{
    (void)key;
    bool channelExists = false;
    if (ChannelName[0] != '#' && ChannelName[0] != '&')
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
                std::string reply = ERR_BADCHANNELKEY(user.getNick(),serverIP,ChannelName);
                send(user.getSocket(), reply.c_str(), reply.size(), 0);
                return;
            }


            channels[i].addMember(user);
            for (size_t j = 0; j < channels[i].getMembers().size(); j++)
            {

                std::string reply = RPL_JOIN(user.getNick(), user.getUserName(), ChannelName, user.getIpAddress());
                send(channels[i].getMembers()[j].getSocket(), reply.c_str(), reply.size(), 0);
                std::cerr << "reply: " << reply << std::endl;
            }
            std::string users ;
            for (size_t j = 0; j < channels[i].getMembers().size(); j++)
            {
                if (channels[i].isoperator(channels[i].getMembers()[j]))
                    users += "@" + channels[i].getMembers()[j].getNick() + " ";
                else
                    users += channels[i].getMembers()[j].getNick() + " ";
            }
            std::string reply = RPL_NAMREPLY(serverIP, users ,ChannelName,user.getNick());
            send(user.getSocket(), reply.c_str(), reply.size(), 0);
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
    reply  = RPL_NAMREPLY(serverIP,"@" + user.getNick() + " ", ChannelName, user.getNick());
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
    for (size_t j = 0; j < users.size(); j++)
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
        reply = ERR_NOSUCHCHANNEL(serverIP,receiverchannel,sender.getNick());
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
 

/*
NICK tamago2
USER tamago2 0 * tamago2
PONG 

:*.freenode.net 696 tamago2 #testing556 k * :You must specify a parameter for the key mode. Syntax: <key>.
":" server_ip 696 nick channel char "*" " :You must specify a parameter for the key mode. Syntax: <key>."
*/