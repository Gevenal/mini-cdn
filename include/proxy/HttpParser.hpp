#pragma once
#include <string>

namespace proxy
{
    struct HttpRequest
    {
        std::string method;
        std::string url;
        std::string host;
        std::string path;
        std::string http_version;
        unsigned short port = 80;
    };
    class HttpParser
    {
    public:
        static HttpRequest parse(const std::string &raw_request);
    };
}