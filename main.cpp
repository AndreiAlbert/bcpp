#include "include/http_request_parser.hpp"
#include "include/http_server.hpp"
#include <thread>

int main() {
    HttpServer h(8080, std::thread::hardware_concurrency());
    h.router.add_route(RequestMethod::GET, "/hello", [](const HttpRequest& _) {
        return "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello, World!";
    });
    h.start();
    return 0;
}
