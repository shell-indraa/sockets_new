#include <cstring>
#include <iostream>
#include <thread>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>


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
        std::cerr << "Error: Master Socket Creation failed.";
        return 1;
    }

    // fill in the details related to the socket
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;   // define the version of socket (ipv4 or 6)
    server_addr.sin_port = htons(PORT); // give the port number in network byte form
    server_addr.sin_addr.s_addr = INADDR_ANY;   // bind to the local ip address and use every network card available

    // bind the socket to the information above like port and ip

    if (bind(server_fd, (sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error: Failed to bind to port Master Socket";
        close(server_fd);
        return 1;
    }

    // start listening for requests 
    if (listen(server_fd, 10) < 0) {
        std::cerr << "Error: Listening Failed";
        close(server_fd);
        return 1;
    }

    std::cout << "Multi-Threaded Server is Listening on PORT: " << PORT << std::endl;

    // accept connection now
    // lets prepare storage for the client stuff
    while (true) {

        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            std::cerr << "Error: Failed to accept client connection";
            close (server_fd);
            return 1;
        }
        std::cout << "New connection accepted! Launching dedicated background thread...\n";
        std::thread worker(handle_client, client_fd);

        // don't do worker.join() because it will block the main thread from going back to listening
        // instead do detach so that os will handle is automatically independent of the main thread
        worker.detach();
    }

    close(server_fd);
    std::cout << "Server Resources Release\n";
    return 0;
}