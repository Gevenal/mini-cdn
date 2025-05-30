#include "../include/proxy/EchoProxy.hpp"
#include "../include/proxy/SocketUtils.hpp"
#include <iostream>
#include <unistd.h>

using namespace proxy;
using namespace net;
EchoProxy::EchoProxy(unsigned short port) : port_(port) {}

void EchoProxy::run()
{
    // 在这里调用你写在 SocketUtils.cpp 里的函数
    int listen_fd = net::create_listen_socket(port_);
    std::cout << "[EchoProxy] Listening on port " << port_ << std::endl;

    while (true)
    {
        int client_fd = net::accept_client(listen_fd);
        if (client_fd < 0)
        {
            // throw std::runtime_error("accept failed");
            continue;
        }
        try
        {
            std::string data = net::read_all(client_fd);
            std::cout << "--------------------" << std::endl;
            std::cout << "[EchoProxy] Received:\n"
                      << data << std::endl;
            std::cout << "--------------------" << std::endl;

            net::write_all(client_fd, data);
            std::cout << "[EchoProxy] Echoed back to client." << std::endl;
        }
        catch (const std::exception &ex)
        {
            std::cerr << "[EchoProxy] Error: " << ex.what() << std::endl;
        }
        ::close(client_fd);
    }
}
