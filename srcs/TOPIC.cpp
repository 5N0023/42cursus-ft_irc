#include "server.hpp"

void server::Topic(size_t i, std::string sBuffer, std::vector<struct pollfd> fds)
{
    int User = getUserBySocket(fds[i].fd);
    try
    {
        std::vector<std::string> vec = split(sBuffer, ' ');
        if (vec.size() < 2)
            throw(channel::channelException(ERR_NEEDMOREPARAMS(users[User].getNick(), serverIP, "TOPIC")));
        if (vec.size() != 2 && vec[2][0] == ':')
            vec[2] = vec[2].substr(1);
        size_t it = 0;
        for (; it < channels.size(); it++)
        {
            if (channels[it].getName() == vec[1])
            {
                if (channels[it].isMember(users[User]))
                {
                    if (vec.size() == 2)
                    {
                        if (channels[it].getHasTopic() == false)
                            throw channel::channelException(RPL_NOTOPIC(serverIP, channels[it].getName(), users[User].getNick()));
                        std::string replay = RPL_TOPICDISPLAY(serverIP, users[User].getNick(), channels[it].getName(), channels[it].getTopic());
                        send(fds[i].fd, replay.c_str(), replay.size(), 0);
                    }
                    else if (channels[it].isoperator(users[User]) || !channels[it].getTopicStrict())
                    {
                        std::string tmp;
                        for (size_t h = 2; h < vec.size(); h++)
                            tmp += " " + vec[h];
                        channels[it].setTopic(tmp);
                        channels[it].setHasTopic(true);
                        std::vector<user> tmpusers = channels[it].getMembers();
                        std::string reply = RPL_SETTOPIC(users[User].getNick(), serverIP, channels[it].getName(), tmp);
                        for (size_t s = 0; s < tmpusers.size(); s++)
                            send(tmpusers[s].getSocket(), reply.c_str(), reply.size(), 0);
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
            throw channel::channelException(ERR_NOSUCHCHANNEL(serverIP, vec[1], users[User].getNick())); // khesni ne3raf channel li jani mena msg
    }
    catch (channel::channelException &e)
    {
        std::string replay = e.what();
        send(fds[i].fd, replay.c_str(), replay.size(), 0);
    }
}