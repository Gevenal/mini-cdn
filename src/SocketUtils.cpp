#include "../include/proxy/SocketUtils.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdexcept>
#include <cstring>
#include <string>

namespace net
{
    int create_listen_socket(uint16_t port)
    {
        int serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocketFd < 0)
        {
            throw std::runtime_error("Socket creation failed: " + std::string(strerror(errno)));
        }
        //  Set port reuse option (SO_REUSEADDR)
        // This allows the server to immediately rebind to the same address and port after shutting down,
        // which is useful for development and quick restarts.

        int optVal = 1;
        if (setsockopt(serverSocketFd, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal)) < 0)
        {
            std::string errorMsg = "setsockopt(SO_REUSEADDR) failed: " + std::string(strerror(errno));
            close(serverSocketFd);
            throw std::runtime_error(errorMsg);
        }
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);
        // bind socket to local host
        if (bind(serverSocketFd, (sockaddr *)&addr, sizeof(addr)) < 0)
        {
            throw std::runtime_error("bind failed");
        }
        // listen
        if (listen(serverSocketFd, SOMAXCONN) < 0)
            throw std::runtime_error("listen failed");
        return serverSocketFd;
    }
    int net::accept_client(int listen_fd)
    {
        // Accept a client connection from the listening socket
        // Returns a new client_fd representing this client

        int client_fd = accept(listen_fd, nullptr, nullptr);
        if (client_fd < 0)
        {
            throw std::runtime_error("accept failed");
        }
        return client_fd;
    }

    /**
     * Reads data sent by the client.
     *
     * Continues reading until the client closes the connection
     * or there is no more data to read.
     *
     * Returns the complete string that was read.
     */

    std::string net::read_all(int client_fd)
    {
        char buffer[4096];
        std::string result;
        ssize_t n;

        while ((n = recv(client_fd, buffer, sizeof(buffer), 0)) > 0)
        {
            result.append(buffer, n);
        }

        if (n < 0)
        {
            throw std::runtime_error("recv failed in net::read_all");
        }
        return result;
    }

    /**
     * Writes data back to the client as-is.
     *
     * Ensures the entire data is sent (since a single send() call
     * might not transmit everything at once).
     */

    void net::write_all(int client_fd, std::string_view data)
    {
        size_t total_sent = 0;
        while (total_sent < data.size())
        {
            ssize_t sent = send(client_fd, data.data() + total_sent, data.size() - total_sent, 0);
            if (sent < 0)
                throw std::runtime_error("send failed");
            total_sent += sent;
        }
    }

    int connect_to_host(const std::string &ip, int port, std::chrono::seconds timeout)
    {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
        {
            throw std::runtime_error("Failed to create socket");
        }

        // Fill sockaddr_in
        sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0)
        {
            ::close(sockfd);
            throw std::runtime_error("Invalid IP address: " + ip);
        }

        // Connect
        if (connect(sockfd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0)
        {
            ::close(sockfd);
            throw std::runtime_error("Connection failed to " + ip + ":" + std::to_string(port));
        }

        return sockfd;
    }
}