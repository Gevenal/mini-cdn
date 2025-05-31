#pragma once

#include <string>
#include <string_view>

namespace net
{
    int create_listen_socket(uint16_t port);
    int accept_client(int listen_fd);
    std::string read_all(int client_fd);
    void write_all(int client_fd, std::string_view data);
    int connect_to_host(const std::string &ip, int port, std::chrono::seconds timeout);

}
