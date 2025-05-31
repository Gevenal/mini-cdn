#pragma once
#include <string>
#include <map>

namespace proxy
{
    struct HttpRequest
    {
        std::string method;
        std::string url;
        std::string host;
        std::string path;
        std::string http_version;
        std::map<std::string, std::string> headers;
        unsigned short port = 80;
    };
    class HttpParser
    {
    public:
        static HttpRequest parse(const std::string &raw_request);
        static std::string serialize(const HttpRequest &request);
    };
}