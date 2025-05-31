#include "../include/proxy/HttpProxy.hpp"
#include "../include/proxy/SocketUtils.hpp"
#include "../include/proxy/HttpParser.hpp"
#include "../include/proxy/Resolver.hpp"
#include <sys/types.h>
#include <sys/socket.h>

#include <iostream>
#include <unistd.h> // close()

using namespace proxy;
using namespace net; // SocketUtils functions

// ---------- ctor ----------
HttpProxy::HttpProxy(unsigned short port) : port_(port) {}

// ---------- run(): accept-loop ----------
void HttpProxy::run()
{
    int listen_fd = net::create_listen_socket(port_);
    std::cout << "[HttpProxy] Listening on 0.0.0.0:" << port_ << std::endl;

    while (true)
    {
        int client_fd = net::accept_client(listen_fd);
        if (client_fd < 0)
        { // should not happen (accept_client throws)
            continue;
        }
        try
        {
            handle_client(client_fd);
        }
        catch (const std::exception &ex)
        {
            std::cerr << "[HttpProxy] Error: " << ex.what() << std::endl;
        }
        ::close(client_fd);
    }
}

/* --- helper: read until end-of-header --- */
static std::string read_request_headers(int fd)
{
    std::string data;
    char buf[1024];
    while (true)
    {
        ssize_t n = recv(fd, buf, sizeof(buf), 0);
        if (n <= 0)
            break;
        data.append(buf, n);
        if (data.find("\r\n\r\n") != std::string::npos)
            break; // header end
    }
    return data;
}

// ---------- handle one request ----------
void HttpProxy::handle_client(int client_fd)
{
    std::string req_raw = read_request_headers(client_fd);
    HttpRequest req = HttpParser::parse(req_raw);

    if (req.host.empty())
        throw std::runtime_error("Invalid HTTP request: missing Host");

    std::string ip = Resolver::instance().resolve(req.host, req.port);
    int origin_fd = net::connect_to_host(ip, req.port, std::chrono::seconds(5));

    std::cout << "[HttpProxy] " << req.method << ' ' << req.host << req.path
              << "  -->  " << ip << ':' << req.port << '\n';

    net::write_all(origin_fd, req_raw); // forward request
    std::string resp_raw = net::read_all(origin_fd);
    net::write_all(client_fd, resp_raw); // relay response

    ::close(origin_fd);
}
