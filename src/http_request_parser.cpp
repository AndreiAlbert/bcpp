#include "../include/http_request_parser.hpp"
#include "../include/logger.hpp"
#include <algorithm>
#include <format>
#include <sstream>

HttpRequestParser::HttpRequestParser() {
    reset();
}

void HttpRequestParser::reset() {
    state_ = ParseState::REQUEST_LINE;
    buffer_.clear();
    request_ = {};
}

HttpRequest HttpRequestParser::get_request() const {
    return request_;
}

bool HttpRequestParser::parse(const std::string& data) {
    buffer_.append(data);

    if (state_ == ParseState::REQUEST_LINE || state_ == ParseState::HEADERS) {
        size_t header_end_pos = buffer_.find("\r\n\r\n");
        if (header_end_pos != std::string::npos) {
            std::string header_data = buffer_.substr(0, header_end_pos);
            parse_headers(header_data);
            
            buffer_.erase(0, header_end_pos + 4);
            state_ = ParseState::BODY;
        }
    }

    if (state_ == ParseState::BODY) {
        size_t content_length = get_content_length();
        if (buffer_.size() >= content_length) {
            request_.body = buffer_.substr(0, content_length);
            buffer_.erase(0, content_length);
            state_ = ParseState::COMPLETE;
        }
    }

    return state_ == ParseState::COMPLETE;
}

void HttpRequestParser::parse_headers(const std::string& header_data) {
    std::istringstream header_stream(header_data);
    std::string line;
    bool is_first_line = true;
    while (std::getline(header_stream, line) && !line.empty() && line[0] != '\r') {
        if (line.back() == '\r') line.pop_back();
        
        if (is_first_line) {
            parse_request_line(line);
            is_first_line = false;
        } else {
            size_t colon_pos = line.find(':');
            if (colon_pos != std::string::npos) {
                // Find the start of the value, skipping whitespace
                size_t value_start = line.find_first_not_of(" \t", colon_pos + 1);
                if (value_start != std::string::npos) {
                    std::string key = line.substr(0, colon_pos);
                    std::string value = line.substr(value_start);
                    request_.headers[key] = value;
                }
            }
        }
    }
}

void HttpRequestParser::parse_request_line(const std::string& line) {
    std::istringstream iss(line);
    std::string method_str;
    iss >> method_str >> request_.full_route >> request_.version;

    static const std::map<std::string, RequestMethod> method_map = {
        {"GET", RequestMethod::GET}, {"POST", RequestMethod::POST}, 
        {"PUT", RequestMethod::PUT}, {"DELETE", RequestMethod::DELETE},
        {"HEAD", RequestMethod::HEAD}, {"OPTIONS", RequestMethod::OPTIONS}
    };
    auto it = method_map.find(method_str);
    if (it != method_map.end()) {
        request_.method = it->second;
    }

    size_t query_pos = request_.full_route.find('?');
    if (query_pos != std::string::npos) {
        request_.route = request_.full_route.substr(0, query_pos);
        // You could add full query param parsing here if needed.
    } else {
        request_.route = request_.full_route;
    }
}

size_t HttpRequestParser::get_content_length() {
    auto it = request_.headers.find("Content-Length");
    if (it != request_.headers.end()) {
        try {
            return std::stoul(it->second);
        } catch (...) {
            // Handle invalid Content-Length header
            return 0;
        }
    }
    return 0;
}
