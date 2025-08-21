#pragma once

#include "thread_pool.hpp"
#include <atomic>
#include <thread>
#include "router.hpp"

using socket_t = int;

class HttpServer {
public:
    HttpServer(int port=8080, size_t number_threads = std::thread::hardware_concurrency());
    ~HttpServer();
    bool start() throw();
    static std::atomic<bool> running;
    Router router;
private:
    int port;
    socket_t socket_fd;
    ThreadPool pool;
    void run();
    void handle_request(int client_fd);
};
