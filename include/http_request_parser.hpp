#pragma once

#include <map>
#include <string>

enum class RequestMethod {
    GET, HEAD, OPTIONS, POST, DELETE, PUT
};

struct HttpRequest {
    RequestMethod method;
    std::string full_route;
    std::string route;
    std::string version;
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> query_params;
    std::map<std::string, std::string> path_params;
    std::string body;
};

class HttpRequestParser {
public:
    HttpRequestParser();
    bool parse(const std::string& data);
    
    HttpRequest get_request() const;
    
    void reset();

private:
    enum class ParseState {
        REQUEST_LINE, HEADERS, BODY, COMPLETE
    };

    void parse_headers(const std::string& header_data);
    void parse_request_line(const std::string& line);
    size_t get_content_length();

    ParseState state_;
    HttpRequest request_;
    std::string buffer_;
};
