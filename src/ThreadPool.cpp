#include "../include/proxy/ThreadPool.hpp"

ThreadPool::ThreadPool(size_t num_threads) : stop_(false)
{
    if (num_threads <= 0)
    {
        throw std::invalid_argument("ThreadPool must have at least one thread");
    }

    for (size_t i = 0; i < num_threads; ++i)
    {
        workers.emplace_back([this]()
                             {
            while (true)
            {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex_);
                    this->condition_.wait(lock, [this]{return this->stop_ || !this->tasks_.empty(); });
                    if (this->stop_ && this->tasks_.empty()) {
                        return;
                    }
                                task = std::move(this->tasks_.front());
                             //   std::cout << "[Thread " << std::this_thread::get_id() << "] Picked up a task" << std::endl;
                                this->tasks_.pop();

                }
                task();
                } });
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        stop_ = true;
    }
    condition_.notify_all();
    for (std::thread &worker : workers)
    {
        if (worker.joinable())
            worker.join();
    }
}