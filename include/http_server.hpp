#pragma once

#include "thread_pool.hpp"
#include <atomic>

using socket_t = int;

class HttpServer {
public:
    HttpServer(int port, size_t number_threads);
    ~HttpServer();
    bool start() throw();
    static std::atomic<bool> running;
private:
    int port;
    socket_t socket_fd;
    ThreadPool pool;
    void run();
    void handle_request(int client_fd);
};
