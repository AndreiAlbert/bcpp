#pragma once

#include "router.hpp"
#include "event_loop.hpp"
#include <atomic>
#include <thread>
#include <vector>
#include <memory>

class HttpServer {
public:
    HttpServer(int port = 8080, size_t number_threads = std::thread::hardware_concurrency());
    ~HttpServer();
    bool start();
    void set_keep_alive_timeout(int seconds);

    static std::atomic<bool> running;
    static int socket_fd;
    Router router;

private:
    void run();

    int port;
    int keep_alive_timeout;
    size_t next_loop;
    std::vector<std::unique_ptr<EventLoop>> event_loops;
    std::vector<std::thread> threads;
};
