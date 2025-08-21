#include "../include/http_status_code.hpp"
#include <unordered_map>

HttpStatus::HttpStatus(HttpStatusCode code): code_(code) {}

HttpStatus::HttpStatus(unsigned int code) {
    static const std::unordered_map<unsigned int, HttpStatusCode> code_map = {
        {100, HttpStatusCode::Continue},
        {200, HttpStatusCode::OK},
        {201, HttpStatusCode::Created},
        {202, HttpStatusCode::Accepted},
        {203, HttpStatusCode::NonAuthoritativeInformation},
        {204, HttpStatusCode::NoContent},
        {301, HttpStatusCode::MovedPermanently},
        {302, HttpStatusCode::Found},
        {400, HttpStatusCode::BadRequest},
        {401, HttpStatusCode::Unauthorized},
        {403, HttpStatusCode::Forbidden},
        {404, HttpStatusCode::NotFound},
        {500, HttpStatusCode::InternalServerError},
        {501, HttpStatusCode::NotImplemented}
    };
    auto it = code_map.find(code);
    if (it == code_map.end()) {
        code_ = HttpStatusCode::InternalServerError;
    } else {
        code_ = it->second;
    }
}

HttpStatusCode HttpStatus::get_status() const { 
    return code_;
}

unsigned int HttpStatus::get_status_as_code() const {
    return static_cast<unsigned int>(code_);
}

void HttpStatus::set_status(HttpStatusCode code) {
    code_ = code;
}

void HttpStatus::set_status(unsigned int code) {
    code_ = static_cast<HttpStatusCode>(code);
}

std::string HttpStatus::as_string() const {
    static const std::unordered_map<HttpStatusCode, std::string> status_text = {
        {HttpStatusCode::Continue, "100 Continue"},
        {HttpStatusCode::OK, "200 OK"},
        {HttpStatusCode::Created, "201 Created"},
        {HttpStatusCode::Accepted, "202 Accepted"},
        {HttpStatusCode::NonAuthoritativeInformation, "203 Non-Authoritative Information"},
        {HttpStatusCode::NoContent, "204 No Content"},
        {HttpStatusCode::MovedPermanently, "301 Moved Permanently"},
        {HttpStatusCode::Found, "302 Found"},
        {HttpStatusCode::BadRequest, "400 Bad Request"},
        {HttpStatusCode::Unauthorized, "401 Unauthorized"},
        {HttpStatusCode::Forbidden, "403 Forbidden"},
        {HttpStatusCode::NotFound, "404 Not Found"},
        {HttpStatusCode::InternalServerError, "500 Internal Server Error"},
        {HttpStatusCode::NotImplemented, "501 Not Implemented"}
    };

    auto it = status_text.find(code_);
    if (it != status_text.end()) {
        return it->second;
    }
    return std::to_string(static_cast<unsigned int>(code_)) + " Unknown Status";
}

bool HttpStatus::is_informational() const {
    auto as_number = static_cast<unsigned int>(code_);
    return (as_number >= 100 && as_number < 200);
}

bool HttpStatus::is_success() const {
    auto as_number = static_cast<unsigned int>(code_);
    return (as_number >= 200 && as_number < 300);
}

bool HttpStatus::is_redirection() const {
    auto as_number = static_cast<unsigned int>(code_);
    return (as_number >= 300 && as_number < 400);
}

bool HttpStatus::is_client_error() const {
    auto as_number = static_cast<unsigned int>(code_);
    return (as_number >= 400 && as_number < 500);
}

bool HttpStatus::is_server_error() const {
    auto as_number = static_cast<unsigned int>(code_);
    return (as_number >= 500 && as_number < 600);
}
