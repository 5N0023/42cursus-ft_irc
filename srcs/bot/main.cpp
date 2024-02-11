
#include "bot.hpp"
int main()
{
    bot bot("127.0.0.1", 6697, "bot", "bot", "bot");
    bot.connectToServer();
    bot.listenToServerAndRespond();
}
