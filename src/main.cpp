#include <iostream>
#include "../include/proxy/HttpProxy.hpp"
#include "../include/proxy/SocketUtils.hpp"

int main()
{
    proxy::HttpProxy proxy(8080, 10, 5);
    proxy.run(); // run() created listen_fd and pool
    return 0;
}
