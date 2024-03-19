#include "utils.hpp"

size_t ToSize_T(std::string &str)
{
    std::istringstream iss(str);
    size_t result;
    if (!(iss >> result))
        return SIZE_MAX;
    return result;
}
bool has_char(std::string str)
{
    for (size_t i = 0; i < str.size(); i++)
    {
        if (!std::isdigit(str[i]))
            return false;
    }
    return true;
}
std::string getreason(std::vector<std::string> vector, size_t begin)
{
    std::string str = "";
    if (begin == vector.size())
        return str;

    for (size_t i = begin; i < vector.size(); i++)
        str += vector[i] + " ";
    return str;
}
user getUser_str(std::string member, std::vector<user> members)
{
    for (size_t i = 0; i < members.size(); i++)
    {
        if (members[i].getNick() == member && members[i].getRegistered())
        {
            return members[i];
        }
    }
    return user("error", -1);
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
std::map<std::string, std::string> parseChannels(std::string channels, std::string keys)
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
    return chans;
}

std::vector<std::string> split(const std::string &input, char delimiter)
{
    std::vector<std::string> result;
    std::istringstream stream(input);
    std::string token;

    while (std::getline(stream, token, delimiter))
    {
        if (token[0] != '\0')
            result.push_back(token);
    }
    return result;
}

std::string getLocalIP()
{
    std::string localIP = "Unable to find IP address";

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1)
    {
        std::cerr << "Could not create socket.\n";
        return localIP;
    }

    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr("8.8.8.8");
    serv.sin_port = htons(80);

    if (connect(sock, (const struct sockaddr *)&serv, sizeof(serv)) == -1)
    {
        std::cerr << "Connect failed.\n";
    }
    else
    {
        struct sockaddr_in name;
        socklen_t namelen = sizeof(name);
        if (getsockname(sock, (struct sockaddr *)&name, &namelen) == -1)
        {
            std::cerr << "getsockname failed.\n";
        }
        else
        {
            char buffer[INET_ADDRSTRLEN];
            if (inet_ntop(AF_INET, &name.sin_addr, buffer, sizeof(buffer)))
            {
                localIP = std::string(buffer);
            }
            else
            {
                std::cerr << "inet_ntop failed.\n";
            }
        }
    }

    close(sock);
    return localIP;
}