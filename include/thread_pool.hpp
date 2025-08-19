#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
public:
    ThreadPool(size_t number_threads);
    ~ThreadPool();
    void add_task(std::function<void()>);
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex mtx; 
    std::condition_variable cv;
    bool stop = false;
};
