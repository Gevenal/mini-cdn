#include <gtest/gtest.h>
#include "../include/proxy/LruCache.hpp"

TEST(LruCacheTest, BasicPutAndGet)
{
    Cache::LruCache<std::string, std::string> cache(2);
    cache.put("a", "1");
    cache.put("b", "2");

    EXPECT_TRUE(cache.contains("a"));
    EXPECT_TRUE(cache.contains("b"));
    EXPECT_EQ(cache.get("a"), std::optional<std::string>{"1"});
    EXPECT_EQ(cache.get("b"), std::optional<std::string>{"2"});
}

TEST(LruCacheTest, EvictionPolicy)
{
    Cache::LruCache<std::string, std::string> cache(2);
    cache.put("a", "1");
    cache.put("b", "2");
    cache.get("a");      // mark "a" as recently used
    cache.put("c", "3"); // "b" should be evicted

    EXPECT_TRUE(cache.contains("a"));
    EXPECT_FALSE(cache.contains("b"));
    EXPECT_TRUE(cache.contains("c"));
}

TEST(LruCacheTest, OverwriteValue)
{
    Cache::LruCache<std::string, std::string> cache(2);
    cache.put("a", "1");
    cache.put("a", "updated");
    EXPECT_EQ(cache.get("a"), std::optional<std::string>{"updated"});
}
