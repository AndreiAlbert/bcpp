#include "include/http_request_parser.hpp"
#include "include/http_response.hpp"
#include "include/http_server.hpp"
#include <format>
#include <iostream>

int main() {
    HttpServer h;
    h.router.add_route(RequestMethod::GET, "/hello/{id}", [](const HttpRequest& request) {
        HttpResponse response;
        int id = std::stoi(request.get_path_param("id").value_or("-1"));
        std::string value = request.get_query_param("key").value_or("no value");
        std::string response_body = std::format("{{ \"message\": \"id is {} and key is {}\" }}", id, value);
        for(const auto& [key, val]: request.query_params) {
            std::cout << key << '=' << val << std::endl;
        }
        response.set_body(response_body);
        response.set_content_type(MimeType::ApplicationJson);
        response.set_status(HttpStatusCode::OK);
        return response;
    });
    h.start();
    return 0;
}
