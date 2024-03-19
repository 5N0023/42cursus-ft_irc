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