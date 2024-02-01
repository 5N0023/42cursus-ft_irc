#include <iostream>
#include <cstring> // For memset and strerror
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 6667

int main() {
    int server_fd, new_socket; // Socket file descriptors
    struct sockaddr_in address; // Address structure for internet
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0}; // Buffer to store data from the client

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 9800
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    // bind all addresses
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the port 9800
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) { // Listen for incoming connections, backlog of 3
        perror("listen");
        exit(EXIT_FAILURE);
    }

    std::cout << "Listening on port " << PORT << "..." << std::endl;

        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
           // continue; // Continue to the next iteration in case of an error
        }
    while (true) {
        std::cout << "Waiting for connections..." << std::endl;


        memset(buffer, 0, 1024); // Clear the buffer
        ssize_t bytes_read = recv(new_socket, buffer, 1024, 0);
        if (bytes_read == 0) {
            std::cout << "Client disconnected." << std::endl;
            close(new_socket); // Close the client socket
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
                perror("accept");
            // continue; // Continue to the next iteration in case of an error
            }
            continue; // Continue to the next iteration of the loop
        }
        std::cout << "Received " << buffer << " bytes." << std::endl;

    }

    // Ideally, we would never reach here in a continuous server
    close(server_fd); // Close the server file descriptor

    return 0;
}
