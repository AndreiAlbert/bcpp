#include "../include/connection.hpp"
#include "../include/logger.hpp"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <sys/socket.h>
#include <unistd.h>
#include <format>

Connection::Connection(int client_fd, Router& router)
    : client_fd(client_fd), router(router), state(ConnectionStatus::READING), keep_alive(false) {
    update_last_activity();
}


Connection::~Connection() {
    if(client_fd) {
        close(client_fd);
    }
}

void Connection::update_last_activity() {
    last_activity = std::chrono::steady_clock::now();
}

bool Connection::is_timed_out(int time_seconds) const {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - last_activity);
    return duration.count() > time_seconds;
}

ConnectionStatus Connection::get_state() const {
    return state;
}

void Connection::handle_read() {
    update_last_activity();
    constexpr size_t BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
    if(bytes_received > 0) {
        read_buffer.append(buffer, bytes_received);
        if(parser.parse(read_buffer)) {
            process_request();
        }
    } else if(bytes_received == 0) {
        state = ConnectionStatus::CLOSING;
    } else {
        state = ConnectionStatus::CLOSING;
    }
}

void Connection::handle_write() {
    update_last_activity();
    ssize_t bytes_sent = send(client_fd, write_buffer.c_str(), write_buffer.size(), 0);

    if (bytes_sent > 0) {
        write_buffer.erase(0, bytes_sent);
        if (write_buffer.empty()) {
            if(keep_alive) {
                parser.reset();
                read_buffer.clear();
                state = ConnectionStatus::READING;
            } else {
                state = ConnectionStatus::CLOSING;
            }
        }
    } else {
        state = ConnectionStatus::CLOSING;
    }
}

void Connection::process_request() {
    HttpRequest request = parser.get_request();
    HttpResponse response;

    auto handler = router.match_route(request.method, request.route, request);
    keep_alive = should_keep_alive(request);
    if (handler.has_value()) {
        response = handler.value()(request);
    } else {
        response.set_status(HttpStatusCode::NotFound);
        response.set_content_type(MimeType::TextPlain);
        response.set_body("Route not found");
    }
    if(keep_alive) {
        response.set_header("Connection", "keep-alive");
    }else {
        response.set_header("Connection", "close");
    }
    write_buffer = response.to_string();
    state = ConnectionStatus::WRITING;
    Logger::get_instance().info(std::format("Handled request for client {}", client_fd));
}

bool Connection::should_keep_alive(const HttpRequest& request) {
    auto it = request.headers.find("Connection");
    if(it != request.headers.end()) {
        std::string conn_header = it->second;
        std::transform(conn_header.begin(), conn_header.end(), conn_header.begin(), ::tolower);
        if(conn_header == "close") {
        Logger::get_instance().info("Connection header set to close");
            return false;
        }
    }
    if(request.version == "HTTP/1.1") {
        Logger::get_instance().info("Http 1.1 detected. Defaulting to keep-alive");
        return true;
    }
    return false;
}

