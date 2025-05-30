#include <iostream>
#include "../include/proxy/EchoProxy.hpp"
int main()
{
    std::cout << "Mini-CDN started\n";
    proxy::EchoProxy server(8080);
    server.run();
    return 0;
}
