#pragma once

#include "http_request_parser.hpp"
#include "router.hpp"
#include <chrono>

enum class ConnectionStatus {
    READING,
    WRITING, 
    CLOSING
};

class Connection {
public:
    Connection(int client_fd, Router& router);
    ~Connection();

    void handle_read();
    void handle_write();

    bool is_timed_out(int time_seconds) const;
    void update_last_activity();

    int get_client_fd() const;
    ConnectionStatus get_state() const;
private:
    void process_request();
    static bool should_keep_alive(const HttpRequest& request);
    int client_fd;
    Router& router;
    ConnectionStatus state;
    bool keep_alive;
    HttpRequestParser parser;
    std::string read_buffer;
    std::string write_buffer;
    std::chrono::steady_clock::time_point last_activity;
};
