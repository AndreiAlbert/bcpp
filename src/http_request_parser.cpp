#include "../include/http_request_parser.hpp"
#include "../include/logger.hpp"
#include <algorithm>
#include <format>
#include <optional>
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
        std::string query_string = request_.full_route.substr(query_pos + 1);
        parse_query_params(query_string);
    } else {
        request_.route = request_.full_route;
    }
}

void HttpRequestParser::parse_query_params(const std::string& query_string) {
    if(query_string.empty()) {
        return;
    }
    std::istringstream ss(query_string);
    std::string pair;
    while (std::getline(ss, pair, '&')) {
        if(pair.empty()) {
            continue;
        }
        size_t eq_pos = pair.find('=');
        std::string key, value;
        if(eq_pos != std::string::npos) {
            key = pair.substr(0, eq_pos);
            value = pair.substr(eq_pos + 1);
        } else {
            key = pair;
            value = "";
        }
        key = url_decode(key);
        value = url_decode(value);
        request_.query_params[key] = value;
    }
}

std::string HttpRequestParser::url_decode(const std::string& encoded) {
    std::string decoded;
    decoded.reserve(encoded.size());
    for (size_t i = 0; i < encoded.size(); ++i) {
        if (encoded[i] == '%' && i + 2 < encoded.size()) {
            std::string hex = encoded.substr(i + 1, 2);
            
            try {
                int value = std::stoi(hex, nullptr, 16);
                decoded += static_cast<char>(value);
                i += 2; 
            } catch (const std::exception&) {
                decoded += encoded[i];
            }
        } else if (encoded[i] == '+') {
            decoded += ' ';
        } else {
            decoded += encoded[i];
        }
    }
    return decoded;
}

size_t HttpRequestParser::get_content_length() {
    auto it = request_.headers.find("Content-Length");
    if (it != request_.headers.end()) {
        try {
            return std::stoul(it->second);
        } catch (...) {
            return 0;
        }
    }
    return 0;
}

std::optional<std::string> HttpRequest::get_query_param(const std::string& key) const {
    auto it = query_params.find(key);
    if(it == query_params.end()) {
        return std::nullopt;
    }
    return std::make_optional(it->second);
}


std::optional<std::string> HttpRequest::get_path_param(const std::string& key) const {
    auto it = path_params.find(key);
    if(it == path_params.end()) {
        return std::nullopt;
    }
    return std::make_optional(it->second);
}
