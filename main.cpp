#include "include/http_request_parser.hpp"
#include "include/http_response.hpp"
#include "include/http_server.hpp"
#include <format>

int main() {
    HttpServer h;
    h.router.add_route(RequestMethod::GET, "/hello/{id}", [](const HttpRequest& request) {
        HttpResponse response;
        int id  = std::stoi(request.path_params.at("id"));
        std::string response_body = std::format("{{ \"message\": \"id is {}\" }}", id);
        response.set_body(response_body);
        response.set_content_type(MimeType::ApplicationJson);
        response.set_status(HttpStatusCode::OK);
        return response;
    });
    h.start();
    return 0;
}
