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
        // 2. 设置端口重用 (SO_REUSEADDR)
        // 这允许服务器在关闭后立即重新绑定到相同的地址和端口，
        // 对于开发和快速重启非常有用。
        int optVal = 1;
        if (setsockopt(serverSocketFd, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal)) < 0)
        {
            std::string errorMsg = "setsockopt(SO_REUSEADDR) failed: " + std::string(strerror(errno));
            close(serverSocketFd); // 清理已创建的 socket
            throw std::runtime_error(errorMsg);
        }
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);
        // 绑定 socket 到本地端口
        if (bind(serverSocketFd, (sockaddr *)&addr, sizeof(addr)) < 0)
        {
            throw std::runtime_error("bind failed");
        }
        // 开始监听传入连接
        if (listen(serverSocketFd, SOMAXCONN) < 0)
            throw std::runtime_error("listen failed");
        return serverSocketFd;
    }
    int net::accept_client(int listen_fd)
    {
        // 从已经监听的 socket 上接受一个客户端连接
        // 返回新的 client_fd，代表这个客户端
        int client_fd = accept(listen_fd, nullptr, nullptr);
        if (client_fd < 0)
        {
            throw std::runtime_error("accept failed");
        }
        return client_fd;
    }

    /**
     * 读取客户端发来的数据

一直读，直到客户端关闭连接或没有更多数据可读

最终返回读到的完整字符串
    */
    std::string net::read_all(int client_fd)
    {
        char buffer[4096];
        std::string result;
        ssize_t n;
        // 在已建立连接的套接字上接收数据
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
     * 把数据原样写回客户端

确保完整写入（可能 send() 一次发不完）
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