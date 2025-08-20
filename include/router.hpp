#pragma once

#include "http_request_parser.hpp"
#include <functional>
#include <optional>
#include <unordered_map>
#include <utility>


struct RouteHash {
    size_t operator() (const std::pair<RequestMethod, std::string>& pair) const;
};

using route = std::pair<RequestMethod, std::string>;
using routes_iterator = std::unordered_map<route, std::function<std::string(const HttpRequest&)>, RouteHash>::iterator;

class Router {
private:
    std::unordered_map<route, std::function<std::string(const HttpRequest&)>, RouteHash> routes;
public:
    Router() = default;
    void add_route(RequestMethod method, const std::string& route, std::function<std::string(const HttpRequest&)> handler);
    std::optional<routes_iterator> get_route(route route);
};
