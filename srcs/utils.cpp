/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hznagui <hznagui@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 11:08:44 by hznagui           #+#    #+#             */
/*   Updated: 2024/02/28 15:16:03 by hznagui          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"


std::string getreason(std::vector<std::string> vector ,int begin)
{
    std::string str="";
    vector[begin]=vector[begin].substr(1);
    for(size_t i=begin;i<vector.size();i++)
        str += vector[i]+" ";
    return str;
}
user getUser_str(std::string member,std::vector<user> members)
{
    for (size_t i = 0; i < members.size(); i++)
    {
        if (members[i].getNick() == member)
        {
            return members[i];
        }
    }
    return user("error",-1);
}
std::vector<std::string> splitCommand(std::string command)
{
    std::vector<std::string> commandVec;
    std::string cmd;
    for (size_t i = 0; i < command.length(); i++)
    {
        if (command[i] == ' ')
        {
            commandVec.push_back(cmd);
            cmd = "";
        }
        else
        {
            cmd += command[i];
        }
    }
    commandVec.push_back(cmd);
    return commandVec;
}
std::map<std::string, std::string> parseChannels(std::string channels,std::string keys)
{
    std::map<std::string, std::string> chans;
    std::vector<std::string> chansVec;
    std::vector<std::string> keysVec;
    std::string chan;
    std::string key;
    for (size_t i = 0; i < channels.length(); i++)
    {
        if (channels[i] == ',')
        {
            chansVec.push_back(chan);
            chan = "";
        }
        else if (i != channels.length() - 1)
        {
            chan += channels[i];
        }
        else if (i == channels.length() - 1)
        {
            chan += channels[i];
            chansVec.push_back(chan);
        }
    }

    for (size_t i = 0; i < keys.length(); i++)
    {
        if (keys[i] == ',' || i == keys.length())
        {
            keysVec.push_back(key);
            key = "";
        }
        else if (i != keys.length() - 1)
        {
            key += keys[i];
        }
        else if (i == keys.length() - 1)
        {
            key += keys[i];
            keysVec.push_back(key);
        }
    }
    for (size_t i = 0; i < chansVec.size(); i++)
    {
        if (i >= keysVec.size())
        {
            keysVec.push_back("");
        }
        chans[chansVec[i]] = keysVec[i];
    }
    // for (size_t i = 0; i < chansVec.size(); i++)
    // {
    //     std::cerr << "channel :" << channels[i] << " key : " << keysVec[i] << std::endl;
    // }
    return chans;
}

std::vector<std::string> split(const std::string& input, char delimiter) {
    std::vector<std::string> result;
    std::istringstream stream(input);
    std::string token;

    while (std::getline(stream, token, delimiter)) {
        if (token [0] != '\0')
            result.push_back(token);
    }

    return result;
}