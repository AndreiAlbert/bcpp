#pragma once

#include <string>

enum class HttpStatusCode {
    Continue = 100,
    OK = 200,
    Created = 201,
    Accepted = 202,
    NonAuthoritativeInformation = 203,
    NoContent = 204,
    MovedPermanently = 301,
    Found = 302,
    BadRequest = 400,
    Unauthorized = 401,
    Forbidden = 403,
    NotFound = 404,
    InternalServerError = 500,
    NotImplemented = 501
};

class HttpStatus {
public:
    HttpStatus(HttpStatusCode code = HttpStatusCode::OK);
    HttpStatus(unsigned int code);

    HttpStatusCode get_status() const;
    unsigned int get_status_as_code() const;

    std::string as_string() const;

    void set_status(HttpStatusCode code);
    void set_status(unsigned int code);

    bool is_informational() const;
    bool is_success() const;
    bool is_redirection() const;
    bool is_client_error() const;
    bool is_server_error() const;
private:
    HttpStatusCode code_;
};
