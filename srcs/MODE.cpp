#include "server.hpp"

void server::Mode(size_t i, std::string sBuffer, std::vector<struct pollfd> fds)
{
    int User = getUserBySocket(fds[i].fd);
    try
    {
        std::vector<std::string> vec = split(sBuffer, ' ');
        if (vec.size() < 3)
            throw(channel::channelException(ERR_NEEDMOREPARAMS(users[User].getNick(), serverIP, "MODE")));
        size_t it = 0;
        for (; it < channels.size(); it++)
        {
            if (channels[it].getName() == vec[1])
            {
                if (channels[it].isMember(users[User]))
                {
                    if (channels[it].isoperator(users[User]))
                    {
                        if (vec[2][0] != '+' && vec[2][0] != '-')
                            throw channel::channelException(ERR_UNKNOWNMODE(users[User].getNick(), serverIP, channels[it].getName(), vec[0][0]));
                        bool positive = (vec[2][0] == '-' ? false : true);
                        std::string ret;
                        size_t arg = 3;
                        ret += vec[2][0];
                        for (size_t k = 1; k < vec[2].size(); k++)
                        {
                            try
                            {
                                if (vec[2][k] == 'i')
                                {
                                    if (positive != channels[it].getMode())
                                    {
                                        channels[it].setMode(positive);
                                        ret += 'i';
                                    }
                                }
                                else if (vec[2][k] == 't')
                                {
                                    if (positive != channels[it].getTopicStrict())
                                    {
                                        channels[it].setTopicStrict(positive);
                                        ret += 't';
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
                                    else if (vec.size() > arg && (positive && !channels[it].getHaskey()))
                                    {
                                        channels[it].setKey(vec[arg]);
                                        ret += 'k';
                                        arg++;
                                        channels[it].setHaskey(positive);
                                    }
                                }
                                else if (vec[2][k] == 'o')
                                {
                                    if (vec.size() > arg)
                                    {
                                        user tmp = getUser_str(vec[arg], users);
                                        if (tmp.getSocket() == -1 && tmp.getIpAddress() == "error")
                                            throw channel::channelException(ERR_NOSUCHNICK(serverIP, vec[arg]));
                                        else if (!channels[it].isMember(tmp) || (channels[it].isoperator(tmp) && positive))
                                        {
                                        }
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
                                    if (positive && vec.size() > arg && has_char(vec[arg]) && (channels[it].getLimit() != ToSize_T(vec[arg]) || !channels[it].getHasLimit()))
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
                                    throw channel::channelException(ERR_UNKNOWNMODE(users[User].getNick(), serverIP, channels[it].getName(), vec[2][k]));
                            }
                            catch (channel::channelException &e)
                            {
                                std::string replay = e.what();
                                send(fds[i].fd, replay.c_str(), replay.size(), 0);
                            }
                        }
                        if (ret.size() > 1)
                        {
                            if (arg != 3)
                            {
                                for (size_t hh = 3; arg != hh; hh++)
                                    ret += +" " + vec[hh];
                            }
                            std::vector<user> tmpusers = channels[it].getMembers();
                            std::string reply = RPL_CHANNELMODEIS(users[User].getNick(), serverIP, channels[it].getName(), ret);
                            for (size_t s = 0; s < tmpusers.size(); s++)
                                send(tmpusers[s].getSocket(), reply.c_str(), reply.size(), 0);
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
            throw channel::channelException(ERR_NOSUCHCHANNEL(serverIP, vec[1], users[User].getNick()));
    }
    catch (channel::channelException &e)
    {
        std::string replay = e.what();
        send(fds[i].fd, replay.c_str(), replay.size(), 0);
    }   
}
