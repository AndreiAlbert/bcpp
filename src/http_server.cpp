#include "../include/http_server.hpp"
#include "../include/logger.hpp"
#include "../include/http_request_parser.hpp"
#include <asm-generic/socket.h>
#include <csignal>
#include <cstring>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <format>
#include <utility>

std::atomic<bool> HttpServer::running(true);

void signal_handler(int signal) {
    if (signal == SIGINT) {
        Logger::get_instance().info("Received SIGINT, initiating shutdown");
        HttpServer::running = false;
    }
}

HttpServer::HttpServer(int port, size_t number_threads): router(Router()) ,port(port), socket_fd(-1), pool(number_threads) {
    struct sigaction sa; 
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, nullptr) < 0) {
        Logger::get_instance().error(std::format("Failed to set SIGINT handler: {}", strerror(errno)));
    }
}

HttpServer::~HttpServer() {
    if (socket_fd > 0) {
        Logger::get_instance().info(std::format("Closing server"));
        close(socket_fd);
        socket_fd = -1;
    }
}

bool HttpServer::start() throw() {
    Logger& logger = Logger::get_instance();
    logger.set_level(LogLevel::INFO);
    try {
        socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd < 0) {
            logger.error(std::format("socket creation failed: {}", strerror(errno)));
            throw std::runtime_error("Failed to create socket");
        }
        logger.info("socket created succesfully");
        int opt = 1;
        if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            logger.error(std::format("setsockopt failed: {}", strerror(errno)));
            close(socket_fd);
            throw std::runtime_error("Failed to set socket option");
        }
        struct sockaddr_in server_addr = {};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        server_addr.sin_addr.s_addr = INADDR_ANY;
        if (bind(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            logger.error(std::format("bind failed on port {}: ", port, strerror(errno)));
            close(socket_fd);
            throw std::runtime_error(std::format("failed to bind on port: {}", std::to_string(port)));
        }
        logger.info(std::format("Bound to port {}", port));
        if (listen(socket_fd, 128) < 0) {
            logger.error(std::format("Listen failed: {}", (strerror(errno))));
            close(socket_fd);
            throw std::runtime_error("failed to listen on socket");
        }
        logger.info(std::format("Server listening on port {}", port));
        run();
        return true;
    } catch (const std::exception& e) {
        logger.error(std::format("server failed on start: {}", e.what()));
    } catch (...) {
        logger.error("server failed: unkown error");
        return false;
    }
    return true;
}

void HttpServer::run() {
    Logger& logger = Logger::get_instance();
    while (running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(socket_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0 && running) {
            logger.error(std::format("Accept failed: {}", strerror(errno)));
            continue; 
        }
        pool.add_task([this, client_fd] {
            handle_request(client_fd);
            close(client_fd);
        });
    }
}

void HttpServer::handle_request(int client_fd) {
    if (!running) {
        return;
    }
    HttpRequest request = HttpRequestParser::parse(client_fd);
    auto route_key = std::make_pair(request.method, request.route);
    auto routes_it = router.get_route(route_key);
    std::string response;
    if (routes_it.has_value()) {
        response = routes_it.value()->second(request).to_string(); 
    } else {
        response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nRoute not found";
    }
    ssize_t bytes_sent = send(client_fd, response.c_str(), response.size() , 0) ;
    if (bytes_sent < 0) {
        Logger::get_instance().error(std::format("Send failed for client {}: {}", client_fd, strerror(errno)));
    } else if (bytes_sent < static_cast<ssize_t>(strlen(response.c_str()))) {
        Logger::get_instance().warning(std::format("Incomplete send for client {}: sent {} bytes", client_fd, bytes_sent));
    } else {
        Logger::get_instance().info(std::format("Handled client request: {}", client_fd));
    }
}
