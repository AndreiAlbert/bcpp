#include "include/http_request_parser.hpp"
#include "include/http_response.hpp"
#include "include/http_server.hpp"

int main() {
    HttpServer h;
    h.router.add_route(RequestMethod::GET, "/hello", [](const HttpRequest& _) {
        HttpResponse response;
        response.set_body("{\"message\": \"hei there\"}");
        response.set_content_type(MimeType::ApplicationJson);
        response.set_status(HttpStatusCode::OK);
        return response;
    });
    h.start();
    return 0;
}
