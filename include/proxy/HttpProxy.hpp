#ifndef HTTP_PROXY_HPP
#define HTTP_PROXY_HPP

#include "LruCache.hpp"
#include "ThreadPool.hpp"
#include "DashEngine.hpp"
#include <string>
#include <map>    // to store the HTTP headers of the cached response.
#include <chrono> // For handling time-related information, like when a response was received or when it expires.
#include <vector> // To store the response body, which can be binary data.
#include <deque>
#include <mutex>

namespace proxy
{

    /**
     * @brief A simple HTTP forward proxy.
     *
     * It accepts incoming HTTP requests from clients, parses them to find
     * the target host/port, then connects to that host, sends the request,
     * and relays back the response.
     */
    class HttpProxy
    {
    public:
        explicit HttpProxy(unsigned short port, size_t cache_max_size_mb, size_t thread_cnt = 5);

        /**
         * @brief Start listening for client connections and handle them.
         */
        void run();

        /**
         * @brief Handle a single client connection (blocking).
         *
         * @param client_fd File descriptor of the accepted client socket.
         */
        void handle_client(int client_fd);

        // Disable copying/moving
        HttpProxy(const HttpProxy &) = delete;
        HttpProxy &operator=(const HttpProxy &) = delete;

    private:
        /**
         * @brief Represents a cached HTTP response.
         * This will be the 'Value' in our LruCache.
         */
        struct ResponseCacheEntry
        {
            std::string status_line;                           // e.g., "HTTP/1.1 200 OK"
            std::map<std::string, std::string> headers;        // e.g., "Content-Type: text/html"
            std::vector<char> body;                            // store the actual HTTP body
            std::chrono::steady_clock::time_point received_at; // the exact time when the proxy received this response from the origin server
            std::chrono::seconds max_age;                      // calculate from Cache-Control: max-age
            std::chrono::steady_clock::time_point expires_at;  // This would be received_at + max_age or parsed from an Expires header.
            std::string etag;                                  // for cache validation
            std::string last_modified;

            // default constructor
            ResponseCacheEntry() : max_age(0) {}
            // A helper method you'd implement in the .cpp. It would check if std::chrono::steady_clock::now() is past the expires_at time
            bool is_stale() const
            {
                return std::chrono::steady_clock::now() > expires_at;
            }
        };
        // 辅助函数，用于从原始响应中解析并创建 CachedHttpResponse
        void process_and_cache_response(const std::string &resp_raw, const std::string &cache_key, int client_fd);

        // 辅助函数，用于将 CachedHttpResponse 序列化回发送给客户端的格式
        // std::vector<char> serialize_cached_response(const CachedHttpResponse& cached_response);
        void send_cached_response(int client_fd, const ResponseCacheEntry &cached);
        unsigned short port_;
        std::deque<double> recent_bandwidths_;   // bandwidth_kbps
        const size_t max_bandwidth_samples_ = 5; // sliding window
        std::mutex bandwidth_mutex_;
        std::unique_ptr<proxy::DashEngine> dash_engine_;
        Cache::LruCache<std::string, ResponseCacheEntry> response_cache_;
        ThreadPool thread_pool_;
    };

} // namespace proxy

#endif // HTTP_PROXY_HPP
