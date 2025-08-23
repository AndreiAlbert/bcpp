#include "../include/http_response.hpp"
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>

void HttpResponse::set_status(HttpStatusCode code) {
    status_.set_status(code);
}

void HttpResponse::set_status(unsigned int code) {
    status_.set_status(code);
}

void HttpResponse::set_status(const HttpStatus& status) {
    status_ = status;
}

void HttpResponse::set_header(const std::string& key, const std::string& value) {
    headers[key] = value;
}

std::optional<std::string> HttpResponse::get_header(const std::string& key) const {
    auto it = headers.find(key);
    if (it == headers.end()) {
        return std::nullopt;
    }
    return std::make_optional(it->second);
}

void HttpResponse::set_content_type(MimeType mime_type) {
    std::string content_type_str = mime_type_to_string(mime_type);
    set_header("Content-Type", content_type_str);
}

void HttpResponse::set_content_type(const std::string& mime_type) {
    set_header("Content-Type", mime_type);
}

void HttpResponse::set_body(const std::string& body) {
    body_ = body;
}

std::string HttpResponse::get_body() const {
    return body_;
}

std::string HttpResponse::to_string() {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << status_.as_string() << "\r\n";
    if (!body_.empty() && headers.find("Content-Length") == headers.end()) {
        headers["Content-Length"] = std::to_string(body_.size());
    }
    for (const auto& [key, val]: headers) {
        oss << key << ": " << val << "\r\n";
    }
    oss << "\r\n";
    if (!body_.empty()) {
        oss << body_;
    }
    return oss.str();
}
