#include <iostream>
#include <cstring> // For memset
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> // For close

std::string getLocalIP() {
    std::string localIP = "Unable to find IP address";

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        std::cerr << "Could not create socket.\n";
        return localIP;
    }

    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr("8.8.8.8");
    serv.sin_port = htons(80);

    if (connect(sock, (const struct sockaddr*)&serv, sizeof(serv)) == -1) {
        std::cerr << "Connect failed.\n";
    } else {
        // Get the local IP address
        struct sockaddr_in name;
        socklen_t namelen = sizeof(name);
        if (getsockname(sock, (struct sockaddr*)&name, &namelen) == -1) {
            std::cerr << "getsockname failed.\n";
        } else {
            char buffer[INET_ADDRSTRLEN];
            if (inet_ntop(AF_INET, &name.sin_addr, buffer, sizeof(buffer))) {
                localIP = std::string(buffer);
            } else {
                std::cerr << "inet_ntop failed.\n";
            }
        }
    }

    close(sock); 
    return localIP;
}

int main() {
    std::cout << "Local IP Address: " << getLocalIP() << std::endl;
    while(1)
    {

    }
    return 0;
}
