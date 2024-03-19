#include "server.hpp"

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <port> <password>\n";
        return -1;
    }
    signal(SIGPIPE, SIG_IGN);
    try
    {
        std::string password = argv[2];
        if (atoi(argv[1]) < 1024 || atoi(argv[1]) > 69698)
        {
            std::cerr << "Port number must be greater than 1024 and less than 69699\n";
            return -1;
        }
        if (password.length() < 1)
        {
            std::cerr << "Password cannot be empty\n";
            return -1;
        }
        for (size_t i = 0; i < password.length(); i++)
            {
                if (password[i] == '\n' || password[i] == ' ')
                {
                    std::cerr << "Password contains invalid characters\n";
                    return -1;
                }
            }
        server ircServer(atoi(argv[1]), password);
        ircServer.run();
    }
    catch (std::exception &e)
    {
        std::cerr << "Error in main: " << e.what() << "\n";
        return -1;
    }
    return 0;
}