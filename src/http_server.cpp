#include "../include/http_server.hpp"
#include "../include/logger.hpp"
#include <csignal>
#include <cstring>
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <format>

std::atomic<bool> HttpServer::running(true);
int HttpServer::socket_fd = -1;

void signal_handler(int signal) {
    if (signal == SIGINT) {
        Logger::get_instance().info("Received SIGINT, initiating shutdown");
        HttpServer::running = false;
        if (HttpServer::socket_fd != -1) {
            shutdown(HttpServer::socket_fd, SHUT_RD);
            close(HttpServer::socket_fd);
            HttpServer::socket_fd = -1; 
        }
    }
}

HttpServer::HttpServer(int port, size_t number_threads)
    : port(port), keep_alive_timeout(10), next_loop(0) {

    for (size_t i = 0; i < number_threads; ++i) {
        event_loops.push_back(std::make_unique<EventLoop>(router, keep_alive_timeout));
    }
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);
}

HttpServer::~HttpServer() {
    if (HttpServer::socket_fd > 0) {
        close(HttpServer::socket_fd);
        HttpServer::socket_fd = -1;
    }
}

void HttpServer::set_keep_alive_timeout(int seconds) {
    keep_alive_timeout = seconds;
}

bool HttpServer::start() {
    Logger& logger = Logger::get_instance();
    logger.set_level(LogLevel::INFO);
    try {
        socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd < 0) throw std::runtime_error("Failed to create socket");
        int opt = 1;
        setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        struct sockaddr_in server_addr = {};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        server_addr.sin_addr.s_addr = INADDR_ANY;
        if (bind(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            throw std::runtime_error(std::format("failed to bind on port: {}", port));
        }
        if (listen(socket_fd, 128) < 0) {
            throw std::runtime_error("failed to listen on socket");
        }
        logger.info(std::format("Server starting... listening on port {}", port));
        for (auto& loop : event_loops) {
            threads.emplace_back([&]() {
                loop->run();
            });
        }
        run();
        for (auto& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
        logger.info("All threads joined. Server shut down cleanly.");

        return true;
    } catch (const std::exception& e) {
        logger.error(std::format("server failed on start: {}", e.what()));
        return false;
    }
}

void HttpServer::run() {
    Logger& logger = Logger::get_instance();
    while (running) {
        int client_fd = accept(socket_fd, nullptr, nullptr);
        if (client_fd < 0) {
            if (running) logger.error(std::format("Accept failed: {}", strerror(errno)));
            continue;
        }

        event_loops[next_loop]->add_connection(client_fd);
        next_loop = (next_loop + 1) % event_loops.size();
    }
    logger.info("Accept loop stopped.");
}
