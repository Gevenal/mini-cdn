#include <gtest/gtest.h>
#include "../include/proxy/HttpParser.hpp"

TEST(HttpParserTest, RelativeUrlWithHostHeader)
{
    std::string raw_request =
        "GET /home HTTP/1.1\r\n"
        "Host: www.example.com:8081\r\n\r\n";

    auto req = proxy::HttpParser::parse(raw_request);
    EXPECT_EQ(req.method, "GET");
    EXPECT_EQ(req.host, "www.example.com");
    EXPECT_EQ(req.port, 8081);
    EXPECT_EQ(req.path, "/home");
}
