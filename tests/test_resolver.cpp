#include <gtest/gtest.h>
#include "../include/proxy/Resolver.hpp"

TEST(ResolverTest, BasicResolution)
{
    std::string ip = proxy::Resolver::instance().resolve("example.com", 80);
    EXPECT_FALSE(ip.empty());
    EXPECT_LE(ip.size(), 15);
    EXPECT_GE(ip.size(), 7);
}

TEST(ResolverTest, NegativeCache)
{
    EXPECT_THROW(
        proxy::Resolver::instance().resolve("this-domain-should-not-exist.tld", 80, std::chrono::seconds{2}),
        std::runtime_error);

    EXPECT_THROW(
        proxy::Resolver::instance().resolve("this-domain-should-not-exist.tld", 80, std::chrono::seconds{2}),
        std::runtime_error);
}
