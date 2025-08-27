#pragma once

#include "http_request_parser.hpp"
#include "http_response.hpp"
#include <functional>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

struct RoutePattern {
    std::string original_pattern; 
    std::vector<std::string> segments;
    std::vector<std::string> param_names;
    std::function<HttpResponse(const HttpRequest&)> handler;
    
    RoutePattern(const std::string& pattern, std::function<HttpResponse(const HttpRequest&)>& handler);
    bool matches(const std::string& path, std::map<std::string, std::string>& params) const;
private:
    void parse_pattern(const std::string& pattern);
    bool is_param_segment(const std::string& segment) const;
    std::string extract_param_name(const std::string& segment) const;
};

struct RouteHash {
    size_t operator() (const std::pair<RequestMethod, std::string>& pair) const;
};

using route = std::pair<RequestMethod, std::string>;
using routes_iterator = std::unordered_map<route, std::function<HttpResponse(const HttpRequest&)>, RouteHash>::iterator;

class Router {
private:
    std::unordered_map<route, std::function<HttpResponse(const HttpRequest&)>, RouteHash> exact_routes;
    std::unordered_map<RequestMethod, std::vector<RoutePattern>> param_routes;
    static bool has_parameters(const std::string& route_pattern);
public:
    Router() = default;
    void add_route(RequestMethod method, const std::string& route, std::function<HttpResponse(const HttpRequest&)> handler);
    std::optional<routes_iterator> get_route(route route);
    std::optional<std::function<HttpResponse(const HttpRequest&)>> match_route(RequestMethod method, const std::string& path, HttpRequest& request);
};
