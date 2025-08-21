#pragma once

#include "http_status_code.hpp"
#include "mime_type.hpp"
#include <optional>
#include <unordered_map>

class HttpResponse {
public:
    HttpResponse() = default;
    ~HttpResponse() = default;

    void set_status(HttpStatusCode code);
    void set_status(unsigned int code);
    void set_status(const HttpStatus& status);

    void set_header(const std::string& key, const std::string& value);
    std::optional<std::string> get_header(const std::string& key) const;

    void set_content_type(MimeType mime_type);
    void set_content_type(const std::string& mime_type);

    void set_body(const std::string& body);
    std::string get_body() const;

    std::string to_string();
private:
    HttpStatus status_;
    std::unordered_map<std::string, std::string> headers;
    std::string body_;    
};
