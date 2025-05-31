#ifndef HTTP_PROXY_HPP
#define HTTP_PROXY_HPP

#include <string>

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
        explicit HttpProxy(unsigned short port);

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
        unsigned short port_;
    };

} // namespace proxy

#endif // HTTP_PROXY_HPP
