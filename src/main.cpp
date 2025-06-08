#include <iostream>
#include "../include/proxy/HttpProxy.hpp"
#include "../include/proxy/SocketUtils.hpp"

int main()
{
    proxy::HttpProxy proxy(8080, 10, 5);
    proxy.run(); // run() 内部已创建 listen_fd 和线程池循环
    return 0;
}
