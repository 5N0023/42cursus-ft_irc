#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h> // For setting socket non-blocking
#include <iostream>
#include <string>

#define CONNECTION_PORT 6697

int main()
{
    struct sockaddr_in server_address;
    char receive_buffer[100];
    int socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_descriptor < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(CONNECTION_PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0)
    {
        perror("Invalid address / Address not supported");
        exit(EXIT_FAILURE);
    }

    if (connect(socket_descriptor, (struct sockaddr*)&server_address, sizeof(server_address)) < 0)
    {
        perror("Couldn't connect with the server");
        exit(EXIT_FAILURE);
    }

    // Set a timeout for recv operations
    std::cout << "Connected to server" << std::endl;

    // fcntl(socket_descriptor, F_SETFL, O_NONBLOCK);
    send(socket_descriptor, "PASS password\r\n", strlen("PASS password\r\n"), 0);
    sleep(1);
    send(socket_descriptor, "NICK bot\r\n", strlen("NICK bot\r\n"), 0);
    sleep(1);
    send(socket_descriptor, "USER bot 0 * :bot\r\n", strlen("USER bot 0 * :bot\r\n"), 0);
    sleep(1);
    std::cout << "registered" << std::endl;
    
    ssize_t received_bytes = recv(socket_descriptor, receive_buffer, sizeof(receive_buffer), 0);
    if (received_bytes < 0) {
        perror("Read operation timed out or failed");
        close(socket_descriptor);
        exit(EXIT_FAILURE);
    }
    
    receive_buffer[received_bytes] = '\0'; // Ensure null-termination
    printf("Message from server: %s\n", receive_buffer);

    close(socket_descriptor);
    return 0;
}
