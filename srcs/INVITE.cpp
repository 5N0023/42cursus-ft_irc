#include "server.hpp"

void server::Invite(size_t i, std::string sBuffer, std::vector<struct pollfd> fds)
{
    int User = getUserBySocket(fds[i].fd);
    try
    {
        std::vector<std::string> vec = split(sBuffer, ' ');
        if (vec.size() < 3)
            throw(channel::channelException(ERR_NEEDMOREPARAMS(users[User].getNick(), serverIP, "INVITE")));
        if (vec[2][0] == ':')
            vec[2] = vec[2].substr(1);
        size_t it = 0;
        for (; it < channels.size(); it++)
        {
            if (channels[it].getName() == vec[2])
            {
                if (channels[it].isMember(users[User]))
                {
                    if (channels[it].isoperator(users[User]))
                    {
                        user tmp1 = getUser_str(vec[1], users);
                        if (tmp1.getSocket() == -1 && tmp1.getIpAddress() == "error")
                            throw channel::channelException(ERR_NOSUCHNICK(serverIP, vec[1]));
                        else
                        {
                            user tmp = getUser_str(vec[1], channels[it].getMembers());
                            if (tmp.getSocket() == -1 && tmp.getIpAddress() == "error")
                            {
                                std::string reply = RPL_INVITE(users[User].getNick(), users[User].getNick(), serverIP, vec[1], vec[2]);
                                channels[it].addInvite(getUser_str(vec[1], users));
                                send(tmp1.getSocket(), reply.c_str(), reply.size(), 0);
                            }
                            else
                                throw channel::channelException(ERR_USERONCHANNEL(serverIP, vec[2], vec[1]));
                        }
                    }
                    else
                        throw channel::channelException(ERR_CHANOPRIVSNEEDED(serverIP, channels[it].getName()));
                }
                else
                    throw channel::channelException(ERR_NOTONCHANNEL(serverIP, channels[it].getName()));
                break;
            }
        }
        if (it == channels.size())
            throw channel::channelException(ERR_NOSUCHCHANNEL(serverIP, vec[2], users[User].getNick())); // khesni ne3raf channel li jani mena msg
    }
    catch (channel::channelException &e)
    {
        std::string replay = e.what();
        send(fds[i].fd, replay.c_str(), replay.size(), 0);
    }

}
