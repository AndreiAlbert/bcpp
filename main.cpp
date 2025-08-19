#include "include/http_server.hpp"
#include <format>
#include <iostream>
#include <thread>

int main() {
    std::cout << std::format("{}\n", std::thread::hardware_concurrency());
    HttpServer h(8080, std::thread::hardware_concurrency());
    h.start();
    return 0;
}
