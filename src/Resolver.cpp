#include "../include/proxy/Resolver.hpp"

#include <stdexcept>
#include <iostream> // For potential debug/error messages, can be replaced by a logger
#include <thread>   // For std::this_thread::sleep_for
#include <vector>

// POSIX/Network headers for getaddrinfo
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>     // For getaddrinfo, addrinfo, gai_strerror
#include <arpa/inet.h> // For inet_ntop
#include <cstring>     // For memset
#include <netinet/in.h>

namespace proxy
{

    // Define constants for caching and retries
    // These could also be private static const members in the class declaration (in .hpp)
    // or configurable if needed.
    const std::chrono::seconds DEFAULT_POSITIVE_TTL{300};      // 5 minutes
    const std::chrono::seconds DEFAULT_NEGATIVE_TTL{60};       // 1 minute (as per assignment.md)
    const int MAX_DNS_RETRIES = 3;                             // As per assignment.md
    const std::string FAILURE_IP_MARKER = "DNS_LOOKUP_FAILED"; // Special marker for negative cache

    Resolver &Resolver::instance()
    {
        // C++11 guarantees thread-safe initialization for static local variables
        static Resolver resolver_instance;
        return resolver_instance;
    }

    std::string Resolver::query_dns(const std::string &hostname, int port)
    {
        addrinfo hints{};
        addrinfo *result_list = nullptr;

        std::memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;       // We are interested in IPv4 addresses only
        hints.ai_socktype = SOCK_STREAM; // TCP

        std::string service_port = std::to_string(port);

        // getaddrinfo is thread-safe
        int gai_error = getaddrinfo(hostname.c_str(), service_port.c_str(), &hints, &result_list);
        if (gai_error != 0)
        {
            // Clean up in case result_list was partially allocated (though unlikely for error returns)
            if (result_list)
            {
                freeaddrinfo(result_list);
            }
            throw std::runtime_error("getaddrinfo failed for " + hostname + ": " + gai_strerror(gai_error));
        }

        std::string ipv4_address_str;
        bool found = false;

        // Iterate through the linked list of results
        for (addrinfo *ptr = result_list; ptr != nullptr; ptr = ptr->ai_next)
        {
            if (ptr->ai_family == AF_INET)
            { // Ensure it's an IPv4 address
                sockaddr_in *ipv4_sockaddr = reinterpret_cast<sockaddr_in *>(ptr->ai_addr);
                char ip_buffer[INET_ADDRSTRLEN]; // Buffer to hold the IPv4 string

                // Convert the binary IP address to a human-readable string
                if (inet_ntop(AF_INET, &(ipv4_sockaddr->sin_addr), ip_buffer, INET_ADDRSTRLEN) != nullptr)
                {
                    ipv4_address_str = ip_buffer;
                    found = true;
                    break; // Found the first IPv4 address, use it
                }
                else
                {
                    // inet_ntop failed, this is unusual
                    // perror("inet_ntop failed"); // For debugging
                }
            }
        }

        freeaddrinfo(result_list); // Always free the memory allocated by getaddrinfo

        if (!found)
        {
            throw std::runtime_error("No IPv4 address found for " + hostname);
        }

        return ipv4_address_str;
    }

    std::string Resolver::resolve(const std::string &hostname,
                                  int port,
                                  std::chrono::seconds timeout)
    {
        auto deadline = std::chrono::steady_clock::now() + timeout;

        // --- 1. Check cache ---
        { // Scope for lock_guard
            std::lock_guard<std::mutex> lock(cache_mutex_);
            auto it = cache_.find(hostname);
            if (it != cache_.end())
            {
                const CacheEntry &entry = it->second;
                if (entry.expires_at > std::chrono::steady_clock::now())
                {
                    if (entry.ip != FAILURE_IP_MARKER)
                    {
                        // std::cout << "[Resolver] Cache hit for " << hostname << ": " << entry.ip << std::endl;
                        return entry.ip; // Positive cache hit
                    }
                    else
                    {
                        // std::cout << "[Resolver] Negative cache hit for " << hostname << std::endl;
                        throw std::runtime_error("Previously failed to resolve " + hostname + " (cached negative result)");
                    }
                }
                else
                {
                    // Entry expired, remove it
                    // std::cout << "[Resolver] Cache expired for " << hostname << std::endl;
                    cache_.erase(it);
                }
            }
        } // Mutex is released here

        // --- 2. Perform DNS query with retries if not found in cache or expired ---
        std::string resolved_ip;
        std::chrono::seconds current_backoff_delay(1); // Initial back-off delay

        for (int attempt = 0; attempt < MAX_DNS_RETRIES; ++attempt)
        {
            if (std::chrono::steady_clock::now() >= deadline)
            {
                // std::cerr << "[Resolver] Timeout before attempt " << attempt + 1 << " for " << hostname << std::endl;
                throw std::runtime_error("DNS resolution timed out for " + hostname);
            }

            // std::cout << "[Resolver] Attempt " << attempt + 1 << " to resolve " << hostname << std::endl;
            try
            {
                resolved_ip = query_dns(hostname, port); // This can block

                // If successful, cache and return
                { // Scope for lock_guard
                    std::lock_guard<std::mutex> lock(cache_mutex_);
                    cache_[hostname] = {resolved_ip, std::chrono::steady_clock::now() + DEFAULT_POSITIVE_TTL};
                    // std::cout << "[Resolver] Resolved and cached " << hostname << " -> " << resolved_ip << std::endl;
                }
                return resolved_ip;
            }
            catch (const std::runtime_error &e)
            {
                // std::cerr << "[Resolver] Attempt " << attempt + 1 << " failed for " << hostname << ": " << e.what() << std::endl;
                if (attempt == MAX_DNS_RETRIES - 1)
                { // Last attempt failed
                    // std::cerr << "[Resolver] All retries failed for " << hostname << std::endl;
                    break; // Break to store negative cache entry and throw
                }

                // Check timeout before sleeping for back-off
                if (std::chrono::steady_clock::now() + current_backoff_delay >= deadline)
                {
                    // std::cerr << "[Resolver] Timeout during back-off for " << hostname << std::endl;
                    // Not enough time for back-off and next attempt, treat as overall timeout
                    // Store negative cache and throw outside the loop
                    break;
                }
                std::this_thread::sleep_for(current_backoff_delay);
                current_backoff_delay *= 2; // Exponential back-off
            }
        }

        // --- 3. All retries failed or timed out during retries ---
        // std::cerr << "[Resolver] Storing negative cache entry for " << hostname << std::endl;
        { // Scope for lock_guard
            std::lock_guard<std::mutex> lock(cache_mutex_);
            cache_[hostname] = {FAILURE_IP_MARKER, std::chrono::steady_clock::now() + DEFAULT_NEGATIVE_TTL};
        }
        throw std::runtime_error("Failed to resolve " + hostname + " after " + std::to_string(MAX_DNS_RETRIES) + " retries or timeout.");
    }

} // namespace proxy
