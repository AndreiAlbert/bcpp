#pragma once

#include "http_request_parser.hpp"
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
    void set_keep_alive_timeout(int seconds);
    void set_max_requests(int max_requests);
private:
    int port;
    int keep_alive_timeout = 30; 
    int max_keep_alive_requests = 100;
    socket_t socket_fd;
    ThreadPool pool;
    void run();
    void handle_connection(int client_fd);
    void handle_request(int client_fd, HttpRequest& request, bool& keep_alive, int& request_count);
    bool should_keep_alive(const HttpRequest& request, int request_count) const;
};
