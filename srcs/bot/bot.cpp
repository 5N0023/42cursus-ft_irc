#include "bot.hpp"

bot::bot(std::string server, int serverPort, std::string nick, std::string user, std::string realname)
{
    this->server = server;
    this->serverPort = serverPort;
    this->nick = nick;
    this->user = user;
    this->realname = realname;
    socket = ::socket(AF_INET, SOCK_STREAM, 0);
}

void bot::connectToServer()
{
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);
    serverAddress.sin_addr.s_addr = inet_addr(server.c_str());
    if (::connect(socket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        std::cerr << "Error connecting to server" << std::endl;
    }
    else
    {
        std::cout << "Connected to server" << std::endl;
    }

}



//

