#include <cstring>
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int PORT = 8080;
int main (int argc, char** argv) {

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

    std::string message = "Hello from the first client\n";
    send(sock_fd, message.c_str(), message.length(), 0);
    std::cout << "Sent: " << message << std::endl;
    char buffer[1024];
    int recvd_bytes = recv(sock_fd, buffer, sizeof(buffer), 0);
    if (recvd_bytes > 0) {
        std::cout << "Server Echoed back " << buffer << std::endl;
    }

    // close the connection
    close(sock_fd);
    std::cout << "Closed socked\n";
    return 0;
}