#include "../include/http_request_parser.hpp"
#include "../include/logger.hpp"
#include <format>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/socket.h>


// TODO changhe to vector<char>
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
    oss << method << " " << path << " "  << version << "\r\n";
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
    iss >> request.method >> request.path >> request.version;
    if (request.method.empty() || request.path.empty() || request.version.empty()) {
        throw std::runtime_error("Invalid request line");
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
