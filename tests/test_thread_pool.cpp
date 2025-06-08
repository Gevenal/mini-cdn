#include <gtest/gtest.h>
#include "proxy/ThreadPool.hpp"
#include <atomic>
#include <chrono>

TEST(ThreadPoolTest, TasksAreExecuted)
{
    ThreadPool pool(4); // create 4 thread pools

    std::atomic<int> counter = 0;

    // submit 100 tasks，each counter + 1
    for (int i = 0; i < 100; ++i)
    {
        pool.enqueue([&counter]
                     {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            counter++; });
    }

    // wait for ensuring every task success
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    EXPECT_EQ(counter.load(), 100);
}
