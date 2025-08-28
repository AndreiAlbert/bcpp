#include "../include/event_loop.hpp"
#include "../include/http_server.hpp"
#include "../include/logger.hpp"
#include <memory>
#include <format>
#include <stdexcept>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>

EventLoop::EventLoop(Router& router, int keep_alive_timeout): router(router), keep_alive_timeout(keep_alive_timeout) {
    epoll_fd = epoll_create1(0);
    if(epoll_fd < 0) {
        throw std::runtime_error("Failed to create epoll file descriptor");
    }
}

EventLoop::~EventLoop() {
    if(epoll_fd) {
        close(epoll_fd);
    }
}

void EventLoop::run() {
    Logger::get_instance().info("Event loop started");
    while(HttpServer::running) {
        handle_events(); 
        check_timeout();
    }
}

void EventLoop::add_connection(int client_fd) {
    int flags = fcntl(client_fd, F_GETFL, 0);
    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
    connections[client_fd] = std::make_unique<Connection>(client_fd, router);
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = client_fd;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) < 0) {
        Logger::get_instance().error("Failed to add client fd to epoll");
        connections.erase(client_fd);
    } else {
        Logger::get_instance().info(std::format("New connection accepted on fd: {}", client_fd));
    }
}

void EventLoop::handle_events() {
    constexpr int MAX_EVENTS = 128;
    struct epoll_event events[MAX_EVENTS];
    int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, 100);
    if(num_events < 0) {
        if(HttpServer::running) {
            Logger::get_instance().error("epoll wait error");
        }
        return;
    }
    for(int i = 0; i < num_events; ++i) {
        int fd = events[i].data.fd;
        auto it = connections.find(fd);
        if(it == connections.end()) {
            continue;
        }
        Connection* conn = it->second.get();
        if(events[i].events & (EPOLLERR | EPOLLHUP)) {
            Logger::get_instance().info(std::format("Closing connection for client {} due to error", fd));
            connections.erase(it);
            continue;
        }
        if(events[i].events & EPOLLIN) {
            conn->handle_read();
        }
        if(events[i].events & EPOLLOUT) {
            conn->handle_write();
        }
        if(conn->get_state() == ConnectionStatus::WRITING) {
            struct epoll_event event;
            event.events = EPOLLOUT | EPOLLET;
            event.data.fd = fd;
            epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
        }else if(conn->get_state() == ConnectionStatus::READING) {
            struct epoll_event event;
            event.events = EPOLLIN | EPOLLET;
            event.data.fd = fd;
            epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
        }else if(conn->get_state() == ConnectionStatus::CLOSING) {
            Logger::get_instance().info(std::format("Closing connection for client: {}", fd));
            connections.erase(it);
        }
    }
}

void EventLoop::check_timeout() {
    std::vector<int> timed_out_fds;
    for(const auto&[client_fd, connection]: connections) {
        if(connection->get_state() == ConnectionStatus::READING && connection->is_timed_out(keep_alive_timeout)) {
            timed_out_fds.push_back(client_fd);
        }
    }
    for(int cfd: timed_out_fds) {
        Logger::get_instance().info(std::format("Connection timed out for client {}", cfd));
        connections.erase(cfd);
    }
}
