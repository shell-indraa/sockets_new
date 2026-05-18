#include <cstring>
#include <iostream>
#include <string>
#include <system_error>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int PORT = 8080;
int main () {

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        std::cerr << "Error: Failed to create socket\n";
        return -1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // convert the string "127..." to bytes in network order and put then into server_addr
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr)<=0) {
        std::cerr << "Invalid Address or Not supported Address\n";
        close(sock_fd);
        return 1;
    }
    // connect to the running server
    // 3 way handshake step

    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error: Connection to server failed. is the server running\n";
        close(sock_fd);
        return 1;
    }

    std::cout << "Connected to server successfully\n";

    std::string message;
    while (true) {

        std::cout << "> ";
        std::getline(std::cin, message);
        if (message == "exit") break;
        if (message.empty()) continue;
        send(sock_fd, message.c_str(), message.length(), 0);
        char buffer[1024];
        int recvd_bytes = recv(sock_fd, buffer, 1024, 0);
        if (recvd_bytes > 0) {
            buffer[recvd_bytes] = 0;
            std::cout << "Server Echoed back: " << buffer << "\n";
        }
        else {
            std::cout << "Server Disconnected\n";
            break;
        }
    }
    // close the connection
    close(sock_fd);
    return 0;
}