#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/event.h>
#include <fcntl.h>


int PORT = 8080;


int main () {

    // make the socket 
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Error: Master Socket Creation failed.\n";
        return 1;
    }
    // make it non blocking
    if (fcntl(server_fd, F_SETFL, O_NONBLOCK)<0) {
        std::cerr << "Failed to make socket Non Blocking.";
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

    // make a kqueue on the kernel
    int kq = kqueue();
    if (kq < 0) {
        std::cerr << "Failed to make Kqueue.\n";
        return 1;
    }

    struct kevent changelist;
    EV_SET(&changelist, server_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);
    if (kevent(kq, &changelist, 1, NULL, 0, NULL) < 0) {
        std::cerr << "Failed to register server socket with kqueue.\n";
        return 1;
    }

    struct kevent event_list[32];
    while (true) {

        int num_events = kevent(kq, NULL, 0, event_list, 32, 0);

        if (num_events < 0) {
            if (errno == EINTR) continue;
            std::cerr << "Error: kevent System Call Failed\n";
            break;
        }
        for (int i=0; i<num_events; i++) {

            // server had some incoming request (new connection request)
            if (event_list[i].ident == static_cast<uintptr_t>(server_fd)) {
                sockaddr_in client_addr{};
                socklen_t client_len = sizeof(client_addr);
                int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
                if (client_fd >= 0) {
                    fcntl(client_fd, F_SETFL, O_NONBLOCK);
                    struct kevent new_client;
                    EV_SET(&new_client, client_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);
                    kevent(kq, &new_client, 1, 0, 0, 0);
                    std::cout << "Engine Registered new Client: " << client_fd << "\n";
                }
            }
            else {
                // one of the client fds was hit, so we have an message incoming
                char buffer[1024];
                memset(buffer, 0, 1024);
                int bytes_received = recv(event_list[i].ident, buffer, 1024, 0);
                if (bytes_received > 0) {
                    buffer[bytes_received] = '\0';
                    std::cout << "FD: " << event_list[i].ident << "Sent: " << buffer << "\n";
                    send(event_list[i].ident, buffer, bytes_received, 0);
                }
                else if (bytes_received == 0) {
                    std::cout << "Client Descriptor: " << event_list[i].ident << " disconnected.\n";
                    close(event_list[i].ident);
                    // Closing the FD automatically purges it from the kernel kqueue!
                }
            }
        }
    }
    close(kq);
    close(server_fd);
    std::cout << "Server Resources Release\n";
    return 0;
}