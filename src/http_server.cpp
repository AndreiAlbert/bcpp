#include "../include/http_server.hpp"
#include "../include/logger.hpp"
#include "../include/http_request_parser.hpp"
#include <algorithm>
#include <asm-generic/socket.h>
#include <cctype>
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

void HttpServer::set_keep_alive_timeout(int seconds) {
    keep_alive_timeout = seconds;
}

void HttpServer::set_max_requests(int max_requets) {
    max_keep_alive_requests = max_requets;
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
        struct timeval tv;
        tv.tv_sec = keep_alive_timeout;
        tv.tv_usec = 0;
        setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
        pool.add_task([this, client_fd] {
            handle_connection(client_fd);
            close(client_fd);
        });
    }
}

void HttpServer::handle_connection(int client_fd) {
    if (!running) {
        return;
    }
    Logger& logger = Logger::get_instance();
    bool keep_alive = true;
    int request_count = 0;
    while (keep_alive && running && request_count < max_keep_alive_requests) {
        try {
            HttpRequest request = HttpRequestParser::parse(client_fd);
            request_count++;
            handle_request(client_fd, request, keep_alive, request_count);
        } catch (const std::exception& e) {
            logger.error(std::format("Error processing request on client {}: {}", client_fd, e.what()));
            break;
        }
    }
    logger.info(std::format("Closing connection for client {} after {} requests", client_fd, request_count));
}

void HttpServer::handle_request(int client_fd, HttpRequest& request, bool& keep_alive, int& request_count) {
    Logger& logger = Logger::get_instance();
    keep_alive = should_keep_alive(request, request_count);
    auto route_key = std::make_pair(request.method, request.route);
    auto route_it = router.get_route(route_key);
    HttpResponse response;
    if (route_it.has_value()) {
        response = route_it.value()->second(request);
    } else {
        response.set_status(HttpStatusCode::NotFound);
        response.set_content_type(MimeType::TextPlain);
        response.set_body("Route not found");
    }
    if (keep_alive) {
        response.set_header("Connection", "keep-alive");
        response.set_header("Keep-Alive", 
            std::format("timeout={}, max={}", keep_alive_timeout, max_keep_alive_requests)); 
    } else {
        response.set_header("Connection", "close");
    }
    std::string response_str = response.to_string();
    ssize_t bytes_sent = send(client_fd, response_str.c_str(), response_str.size(), 0);
    if (bytes_sent < 0) {
        logger.error(std::format("Send failed for client {}: {}", client_fd, strerror(errno)));
        keep_alive = false;
    } else if (bytes_sent < static_cast<ssize_t>(response_str.size())) {
        logger.warning(std::format("Incomplete send for client {}: sent {} bytes", client_fd, bytes_sent));
        keep_alive = false;
    } else {
        logger.info(std::format("Handled request {} for client {}", request_count, client_fd));
    }
}

bool HttpServer::should_keep_alive(const HttpRequest& request, int request_count) const {
    if (request_count >= max_keep_alive_requests) {
        return false;
    }
    auto connection_it = request.headers.find("Connection");
    if (connection_it != request.headers.end()) {
        std::string connection_value = connection_it->second;
        std::transform(connection_value.begin(), connection_value.end(), 
                       connection_value.begin(), 
                       ::tolower);
        if (connection_value == "close") {
            return false;
        } else if (connection_value == "keep-alive") {
            return true;
        }
    }
    if (request.version == "HTTP/1.1") {
        return true;
    }
    return false;
}


