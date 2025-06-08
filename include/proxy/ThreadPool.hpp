#pragma once

#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <cstddef>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>  // For std::function, std::bind
#include <future>      // For std::future, std::packaged_task
#include <memory>      // For std::make_shared, std::shared_ptr
#include <stdexcept>   // For std::runtime_error
#include <type_traits> // For std::invoke_result_t
#include <utility>     // For std::forward, std::move
#include <iostream>
class ThreadPool
{
public:
    /**
     * @brief Constructs a ThreadPool with a fixed number of worker threads.
     * @param num_threads The number of worker threads to create.
     * @throw std::invalid_argument if num_threads is 0.
     */
    explicit ThreadPool(size_t num_threads);

    /**
     * @brief Destructor. Signals worker threads to stop, waits for them to complete
     * their current tasks, and then joins them.
     */
    ~ThreadPool();

    /**
     * @brief Enqueues a task to be executed by a worker thread.
     *
     * @tparam F Type of the callable object (function, lambda, etc.).
     * @tparam Args Types of the arguments to the callable object.
     * @param f The callable object.
     * @param args The arguments to pass to the callable object.
     * @return A std::future representing the eventual result of the task execution.
     * @throw std::runtime_error if the thread pool is stopping.
     */
    template <class F, class... Args>
    auto enqueue(F &&f, Args &&...args)
        -> std::future<std::invoke_result_t<F, Args...>>
    {
        using return_type = std::invoke_result_t<F, Args...>;

        // 用 std::bind 生成 0 参数 callable，C++17 OK
        auto bound = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

        auto task = std::make_shared<std::packaged_task<return_type()>>(std::move(bound));
        std::future<return_type> res = task->get_future();

        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            if (stop_)
                throw std::runtime_error("enqueue on stopped ThreadPool");
            tasks_.emplace([task]
                           { 
               std::cout << "[Thread " << std::this_thread::get_id() << "] Executing task" << std::endl << std::flush;
    (*task)(); });
        }
        condition_.notify_one();
        return res;
    }

    // Prevent copying and moving of the ThreadPool object
    ThreadPool(const ThreadPool &) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;
    ThreadPool(ThreadPool &&) = delete;
    ThreadPool &operator=(ThreadPool &&) = delete;

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;            // Mutex to protect access to the task queue
    std::condition_variable condition_; // Condition variable for task synchronization
    std::atomic<bool> stop_;            // Flag to indicate whether threads should stop
};
#endif