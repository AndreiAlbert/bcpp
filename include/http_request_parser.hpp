#pragma once

#include <map>
#include <string>

enum class RequestMethod {
    GET,
    HEAD,
    OPTIONS,
    POST, 
    DELETE,
    PUT 
};

struct HttpRequest {
    RequestMethod method; 
    std::string path; 
    std::string version;
    std::map<std::string, std::string> headers; 
    std::string body;
    std::string to_string() const;
};


class HttpRequestParser {
public:
    static HttpRequest parse(int client_fd);
    static std::string enum_to_string_method(const RequestMethod& method);
private:
    static void parse_headers(HttpRequest& request, const std::string& header_data);
    static void parse_request_line(HttpRequest& request, const std::string& line);
    static size_t get_content_length(const HttpRequest& request);
    static RequestMethod string_to_enum_method(const std::string& method);
};
