#include "server.hpp"

void server::Kick(size_t i, std::string sBuffer, std::vector<struct pollfd> fds)
{
    int User = getUserBySocket(fds[i].fd);
    try
    {
        std::vector<std::string> vec = split(sBuffer, ' ');
        if (vec.size() < 3)
            throw(channel::channelException(ERR_NEEDMOREPARAMS(users[User].getNick(), serverIP, "KICK")));
        std::vector<std::string> chan = split(vec[1], ','), target = split(vec[2], ',');
        for (size_t y = 0; y < chan.size(); y++)
        {
            size_t it = 0;
            try
            {
                for (; it < channels.size(); it++)
                {
                    if (channels[it].getName() == chan[y])
                    {
                        if (channels[it].isMember(users[User]))
                        {
                            if (channels[it].isoperator(users[User]))
                            {
                                for (size_t x = 0; x < target.size(); x++)
                                {
                                    try
                                    {
                                        user tmp = getUser_str(target[x], channels[it].getMembers());
                                        if (tmp.getSocket() == -1 && tmp.getIpAddress() == "error")
                                            throw channel::channelException(ERR_NOSUCHNICK(serverIP, target[x]));
                                        else
                                        {
                                            std::string reply = RPL_KICK(users[User].getNick(), users[User].getNick(), serverIP, chan[y], target[x], getreason(vec, 3));
                                            std::vector<user> tmpusers = channels[it].getMembers();
                                            for (size_t s = 0; s < tmpusers.size(); s++)
                                                send(tmpusers[s].getSocket(), reply.c_str(), reply.size(), 0);
                                            channels[it].removeMember(tmp, 0);
                                        }
                                    }
                                    catch (channel::channelException &e)
                                    {
                                        std::string replay = e.what();
                                        send(fds[i].fd, replay.c_str(), replay.size(), 0);
                                    }
                                    it = channels.size() + 1;
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
                    throw channel::channelException(ERR_NOSUCHCHANNEL(serverIP, chan[y], users[User].getNick()));
            }
            catch (channel::channelException &e)
            {
                std::string replay = e.what();
                send(fds[i].fd, replay.c_str(), replay.size(), 0);
            }
        }
    }
    catch (channel::channelException &e)
    {
        std::string replay = e.what();
        send(fds[i].fd, replay.c_str(), replay.size(), 0);
    }
}
