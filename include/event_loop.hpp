#pragma once

#include "connection.hpp"
#include "router.hpp"
#include <memory>
#include <unordered_map>
class EventLoop {
public:
    EventLoop(Router& router, int keep_alive_timeout);
    ~EventLoop();

    void run();
    void add_connection(int client_fd);

private:
    void handle_events();
    void check_timeout();
    int epoll_fd;
    Router& router;
    int keep_alive_timeout;
    std::unordered_map<int, std::unique_ptr<Connection>> connections;
};
