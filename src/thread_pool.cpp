#include "../include/thread_pool.hpp"
#include <functional>
#include <mutex>
#include <sstream>
#include <thread>

ThreadPool::ThreadPool(size_t number_threads) {
    for (size_t i = 0; i < number_threads; ++i) {
        workers.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(mtx);
                    cv.wait(lock, [this] { return stop || !tasks.empty(); });
                    if (stop && tasks.empty()) return;
                    task = std::move(tasks.front());
                    tasks.pop();
                }
                std::ostringstream oss;
                oss << std::this_thread::get_id();
                task();
            }
        });
    }
}

void ThreadPool::add_task(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(mtx);
        tasks.push(std::move(task));
    }
    cv.notify_one();
}

ThreadPool::~ThreadPool() {
    {
        std::lock_guard<std::mutex> lock(mtx);
        stop = true;
    }
    cv.notify_all();
    for(auto& worker: workers) {
        worker.join();
    }
}
