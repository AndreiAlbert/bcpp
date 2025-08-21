#include "../include/http_request_parser.hpp"
#include "../include/logger.hpp"
#include <algorithm>
#include <cctype>
#include <format>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/socket.h>


HttpRequest HttpRequestParser::parse(int client_fd) {
    constexpr size_t BUFFER_SIZE = 2048;
    HttpRequest request;
    std::string request_data; 
    bool header_completed = false;
    size_t content_length = 0;
    while(true) {
        char buffer[BUFFER_SIZE];
        ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received < 0) {
            Logger::get_instance().error("Error receiving data from client");
            throw std::runtime_error("Failed to receive data");
        }
        if (bytes_received == 0) {
            break;
        }
        buffer[bytes_received] = '\0';
        request_data.append(buffer, bytes_received);
        if (!header_completed) {
            size_t pos = request_data.find("\r\n\r\n");
            if (pos != std::string::npos) {
                header_completed = true;
                parse_headers(request, request_data.substr(0, pos));
                content_length = get_content_length(request);
                if (content_length == 0) {
                    break;
                }
            }
        }
        if (header_completed && request_data.size() >= content_length + request_data.find("\r\n\r\n") + 4) {
            request.body = request_data.substr(request_data.find("\r\n\r\n") + 4);
            break;
        }
    }
    return request;
}

std::string HttpRequest::to_string() const {
    std::ostringstream oss; 
    oss << HttpRequestParser::enum_to_string_method(method) << " " << full_route << " "  << version << "\r\n";
    for (const auto& [key, val]: headers) {
        oss << key << ": " << val << "\r\n";
    }
    oss << "\r\n" << body;
    return oss.str();
}

void HttpRequestParser::parse_headers(HttpRequest& request, const std::string& header_data) {
    std::istringstream header_stream(header_data);
    std::string line; 
    bool first_line = true;
    while (std::getline(header_stream, line) && line != "\r") {
        if (line.back() == '\r') {
            line.pop_back();
        }
        if (first_line) {
            parse_request_line(request, line);
            first_line = false;
        } else {
            size_t colon_pos = line.find(':');
            if (colon_pos != std::string::npos) {
                std::string key = line.substr(0, colon_pos);
                std::string value = line.substr(colon_pos + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);
                request.headers[key] = value;
            }
            else {
                Logger::get_instance().warning(std::format("Skipping malformed header: {}", line));
            }
        }
    }
}

void HttpRequestParser::parse_request_line(HttpRequest& request, const std::string& line) {
    std::istringstream iss(line);
    std::string method_str, full_route;
    iss >> method_str >> request.full_route >> request.version;
    if (method_str.empty() || request.full_route.empty() || request.version.empty()) {
        throw std::runtime_error("Invalid request line");
    }
    request.method = string_to_enum_method(method_str);
    full_route = request.full_route;

    size_t query_pos = full_route.find('?');
    std::string path; 
    std::string query_params_path;
    if (query_pos == std::string::npos) {
        request.route = request.full_route;
        return;
    }
    request.route = request.full_route.substr(0, query_pos);
    path = full_route.substr(0, query_pos);
    query_params_path = full_route.substr(query_pos + 1);
    std::istringstream query_params_stream(query_params_path);
    std::string param;
    while(std::getline(query_params_stream, param, '&')) {
        size_t eq_pos = param.find('=');
        if (eq_pos != std::string::npos) {
            std::string key = param.substr(0, eq_pos);
            std::string value = param.substr(eq_pos + 1);
            request.query_params[key] = value;
        }else {
            Logger::get_instance().warning(std::format("Skipping malformed query param: {}", param));
        }
    }
}

size_t HttpRequestParser::get_content_length(const HttpRequest& request) {
    auto it = request.headers.find("Content-Length");
    if (it != request.headers.end()) {
        try {
            return std::stoi(it->second);
        } catch(...) {
            throw std::runtime_error("Invalid content length");
        }
    }
    return 0;
}

RequestMethod HttpRequestParser::string_to_enum_method(const std::string& method) {
    static const std::map<std::string, RequestMethod> method_map = {
        {"GET", RequestMethod::GET},
        {"HEAD", RequestMethod::HEAD},
        {"OPTIONS", RequestMethod::OPTIONS},
        {"POST", RequestMethod::POST},
        {"DELETE", RequestMethod::DELETE},
        {"PUT", RequestMethod::PUT},
    };
    std::string method_upper = method;
    std::transform(method_upper.begin(), method_upper.end(), method_upper.begin(), 
                   [](unsigned char c) {return std::toupper(c);});
    auto it = method_map.find(method_upper);
    if (it == method_map.end()) {
        throw std::runtime_error(std::format("Invalid http method: {}", method));
    }
    return it->second;
}

std::string HttpRequestParser::enum_to_string_method(const RequestMethod& method) {
    switch (method) {
    case RequestMethod::GET:
        return "GET";
    case RequestMethod::HEAD:
        return "HEAD";
    case RequestMethod::OPTIONS:
        return "OPTIONS";
    case RequestMethod::POST:
        return "POST";
    case RequestMethod::DELETE:
        return "DELETE";
    case RequestMethod::PUT:
        return "PUT";
    default:
        throw std::runtime_error("Unkown request method");
    }
}
