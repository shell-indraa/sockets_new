#include <cstring>
#include <iostream>
#include <thread>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <poll.h>

int PORT = 8080;

void handle_client(int client_fd) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int recvd_bytes = recv(client_fd, buffer, sizeof(buffer), 0);

        if (recvd_bytes <= 0) {
            std::cout << "Client Disconneted or link dropped, breaking loop\n";
            break;
        }
        std::cout << "Received From Client " << buffer << std::endl;
        send(client_fd, buffer, recvd_bytes, 0);
    }

    //cleanup
    close(client_fd);
}
int main () {

    // make the socket 
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Error: Master Socket Creation failed.\n";
        return 1;
    }

    // fill in the details related to the socket
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;   // define the version of socket (ipv4 or 6)
    server_addr.sin_port = htons(PORT); // give the port number in network byte form
    server_addr.sin_addr.s_addr = INADDR_ANY;   // bind to the local ip address and use every network card available

    // bind the socket to the information above like port and ip
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "setsockopt failed.\n";
        close(server_fd);
        return 1;
    }

    if (bind(server_fd, (sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error: Failed to bind to port Master Socket.\n";
        close(server_fd);
        return 1;
    }

    // start listening for requests 
    if (listen(server_fd, 10) < 0) {
        std::cerr << "Error: Listening Failed.\n";
        close(server_fd);
        return 1;
    }

    std::cout << "Multi-Threaded Server is Listening on PORT: " << PORT << std::endl;

    std::vector<struct pollfd> fds;
    fds.push_back({server_fd, POLLIN, 0});

    // accept connection now
    // lets prepare storage for the client stuff
    while (true) {

        int service = poll(fds.data(), fds.size(), -1);

        if (service < 0) {
            std::cerr << "Error: Poll System Call Failed\n";
            break;
        }
        int current_size = fds.size();
        for (int i=0; i<current_size; i++) {

            if (fds[i].revents & POLLIN) {

                // server had some incoming request (new connection request)
                if (fds[i].fd == server_fd) {

                    sockaddr_in client_addr{};
                    socklen_t client_len = sizeof(client_addr);
                    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
                    if (client_fd >= 0) {
                        fds.push_back({client_fd, POLLIN, 0});
                    }
                }
                else {
                    // one of the client fds was hit, so we have an message incoming
                    char buffer[1024];
                    memset(buffer, 0, 1024);
                    int bytes_received = recv(fds[i].fd, buffer, 1024, 0);
                    if (bytes_received <= 0) {
                        std::cout << "Client Descriptor: " << fds[i].fd << " disconnected.\n";
                        close(fds[i].fd);
                        fds.erase(fds.begin() + i);
                        i--;
                        current_size--;
                    }
                    else {
                        buffer[bytes_received] = '\0';
                        std::cout << "Socket " << fds[i].fd << " Sent: " << buffer << "\n";
                        send(fds[i].fd, buffer, bytes_received, 0);
                    }
                }
            }
        }
    }

    close(server_fd);
    std::cout << "Server Resources Release\n";
    return 0;
}