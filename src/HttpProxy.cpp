#include "../include/proxy/HttpProxy.hpp"
#include "../include/proxy/SocketUtils.hpp"
#include "../include/proxy/HttpParser.hpp"
#include "../include/proxy/Resolver.hpp"
#include <sys/types.h>
#include <sys/socket.h>

#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <unistd.h> // close()

using namespace proxy;
using namespace net; // SocketUtils functions

// ---------- ctor ----------
HttpProxy::HttpProxy(unsigned short port, size_t cache_max_size_mb, size_t thread_cnt) : port_(port),
                                                                                         response_cache_(cache_max_size_mb > 0 ? cache_max_size_mb * 5 : 100),
                                                                                         thread_pool_(thread_cnt)
{
    if (cache_max_size_mb == 0)
    {
        std::cout << "[HttpProxy] Warning: Cache size parameter implies very small or zero capacity. Cache might be ineffective or using default." << std::endl;
    }
    // std::cout << "[HttpProxy] Initialized on port " << port_
    //           << " with estimated cache capacity: " << (cache_max_size_mb > 0 ? cache_max_size_mb * 5 : 100) << " items." << std::endl;
}

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
            std::cerr << "[HttpProxy] accept_client returned an error or invalid fd." << std::endl;
            continue;
        }
        thread_pool_.enqueue([this, client_fd]()
                             {
    try {
         std::cout << "[Thread " << std::this_thread::get_id()
              << "] Handling client_fd = " << client_fd << std::endl;
  
        handle_client(client_fd);
    } catch (const std::exception &ex) {
        std::cerr << "[HttpProxy] Error: " << ex.what() << std::endl;
    }
    ::close(client_fd); });
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
    std::cout << "[Thread " << std::this_thread::get_id()
              << "] Handling client fd = " << client_fd << std::endl;

    std::string req_raw = read_request_headers(client_fd);
    HttpRequest req = HttpParser::parse(req_raw);

    if (req.host.empty())
        throw std::runtime_error("Invalid HTTP request: missing Host");

    std::string cache_key = req.host + req.path;

    // check cache before network
    auto cached = response_cache_.get(cache_key);
    if (cached.has_value())
    {
        if (!cached->is_stale())
        {
            std::cout << "[HttpProxy] Cache HIT: " << cache_key << std::endl;
            send_cached_response(client_fd, *cached);
            return;
        }
        // cache expire, validating it with the origin
        std::cout << "[HttpProxy] Cache EXPIRED: validating with conditional request: " << cache_key << std::endl;
        //  Attach ETag validator header if it exists
        if (!cached->etag.empty())
        {
            req.headers["If-None-Match"] = cached->etag;
        }
        // Attach Last-Modified validator header if it exists
        if (!cached->last_modified.empty())
        {
            req.headers["If-Modified-Since"] = cached->last_modified;
        } // conditional GET" request
        // Serialize the updated HTTP request (with conditional headers)
        // turn the req into HTTP which can be sent
        std::string conditional_req = HttpParser::serialize(req);
        // Connect to the origin server,
        // send the conditional GET request, and read the response.
        std::string ip = Resolver::instance().resolve(req.host, req.port);
        int origin_fd = net::connect_to_host(ip, req.port, std::chrono::seconds(5));
        net::write_all(origin_fd, conditional_req);
        std::string resp_raw = net::read_all(origin_fd);
        ::close(origin_fd);

        if (resp_raw.find("304 Not Modified") != std::string::npos)
        {
            std::cout << "[HttpProxy] Server returned 304: reusing cached response.\n";
            send_cached_response(client_fd, *cached);
            return;
        }

        process_and_cache_response(resp_raw, cache_key, client_fd);
        return;
    }
    std::cout << "[HttpProxy] Cache MISS: " << cache_key << std::endl;

    std::string ip = Resolver::instance().resolve(req.host, req.port);
    int origin_fd = net::connect_to_host(ip, req.port, std::chrono::seconds(5));

    std::cout << "[HttpProxy] " << req.method << ' ' << req.host << req.path
              << "  -->  " << ip << ':' << req.port << '\n';

    net::write_all(origin_fd, req_raw);
    std::string resp_raw = net::read_all(origin_fd);
    ::close(origin_fd);

    // cache and send to client
    process_and_cache_response(resp_raw, cache_key, client_fd);
}

/** ------------------
 * helper method
 * ------------------
 */

void HttpProxy::send_cached_response(int client_fd, const ResponseCacheEntry &cached)
{
    std::string full_response = cached.status_line + "\r\n";
    for (const auto &kv : cached.headers)
    {
        full_response += kv.first + ": " + kv.second + "\r\n";
    }
    full_response += "\r\n";
    full_response.append(cached.body.begin(), cached.body.end());

    net::write_all(client_fd, full_response);
}

void HttpProxy::process_and_cache_response(const std::string &resp_raw, const std::string &cache_key, int client_fd)
{
    auto header_end_pos = resp_raw.find("\r\n\r\n");
    if (header_end_pos == std::string::npos)
    {
        throw std::runtime_error("Invalid HTTP response (no header-body split)");
    }

    std::string status_line, header_block;
    std::vector<char> body;
    std::map<std::string, std::string> header_map;

    status_line = resp_raw.substr(0, resp_raw.find("\r\n"));

    // Corrected header_block extraction
    header_block = resp_raw.substr(resp_raw.find("\r\n") + 2, header_end_pos - (resp_raw.find("\r\n") + 2));

    std::istringstream stream(header_block);
    std::string line;
    while (std::getline(stream, line))
    {
        auto colon_pos = line.find(":");
        if (colon_pos != std::string::npos)
        {
            std::string key = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);
            while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
                value.erase(0, 1);
            header_map[key] = value;
        }
    }

    // Extract the body
    body.assign(resp_raw.begin() + header_end_pos + 4, resp_raw.end());

    // Compute expiration metadata
    std::chrono::steady_clock::time_point expires_at;
    std::chrono::seconds max_age{0};
    auto now = std::chrono::steady_clock::now();

    if (header_map.count("Cache-Control") && header_map["Cache-Control"].find("max-age=") != std::string::npos)
    {
        std::string cc = header_map["Cache-Control"];
        size_t pos = cc.find("max-age=");
        if (pos != std::string::npos)
        {
            int age = std::stoi(cc.substr(pos + 8));
            max_age = std::chrono::seconds(age);
        }
    }
    expires_at = now + max_age;

    // Build and insert the cache entry
    ResponseCacheEntry entry;
    entry.status_line = status_line;
    entry.headers = header_map;
    entry.body = body;
    entry.received_at = now;
    entry.max_age = max_age;
    entry.expires_at = expires_at;
    if (header_map.count("ETag"))
        entry.etag = header_map["ETag"];
    if (header_map.count("Last-Modified"))
        entry.last_modified = header_map["Last-Modified"];

    response_cache_.put(cache_key, entry);
    send_cached_response(client_fd, entry);
}
