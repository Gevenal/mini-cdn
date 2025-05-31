#include <iostream>
#include "../include/proxy/HttpProxy.hpp"
#include "../include/proxy/SocketUtils.hpp"

int main()
{
    proxy::HttpProxy proxy(8080, 10);
    int listen_fd = net::create_listen_socket(8080);

    while (true)
    {
        int client_fd = net::accept_client(listen_fd);
        proxy.handle_client(client_fd);
    }
}
