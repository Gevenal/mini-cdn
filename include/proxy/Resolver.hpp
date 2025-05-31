#ifndef RESOLVER_HPP
#define RESOLVER_HPP

#include <string>
#include <chrono>
#include <unordered_map>
#include <mutex>

namespace proxy
{

    /**
     * @brief Simple synchronous DNS resolver (IPv4 only).
     *
     * Basic usage:
     * ```
     * std::string ip = proxy::Resolver::instance().resolve("example.com", 80);
     * // ip == "93.184.216.34"
     * ```
     *
     * Why a singleton?
     *  - A single shared cache for positive / negative results
     *  - Easy to make thread-safe and to clean up centrally
     *
     * Thread-safety:
     *  - The public API guards the cache with an internal mutex
     */
    class Resolver
    {
    public:
        /** Access to the global singleton instance */
        static Resolver &instance();

        /**
         * Resolve a hostname to an IPv4 string.
         *
         * @param hostname  the name to resolve (e.g. "example.com")
         * @param port      destination port (used by getaddrinfo, usually 80/443)
         * @param timeout   how long to wait before giving up (default: 5 s)
         * @return          IPv4 dotted-quad string, e.g. "93.184.216.34"
         * @throws std::runtime_error if the lookup fails or times out
         */
        std::string resolve(const std::string &hostname,
                            int port,
                            std::chrono::seconds timeout = std::chrono::seconds{5});

        /* non-copyable, non-movable */
        Resolver(const Resolver &) = delete;
        Resolver &operator=(const Resolver &) = delete;

    private:
        Resolver() = default; // private ctor => singleton only

        /* ---------- simple in-memory cache ---------- */
        struct CacheEntry
        {
            std::string ip;
            std::chrono::steady_clock::time_point expires_at;
        };

        std::unordered_map<std::string, CacheEntry> cache_;
        std::mutex cache_mutex_;

        /* Low-level helper that actually calls getaddrinfo() */
        static std::string query_dns(const std::string &hostname, int port);
    };

} // namespace proxy

#endif // RESOLVER_HPP
