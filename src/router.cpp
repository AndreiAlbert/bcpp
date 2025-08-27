#include "../include/router.hpp"
#include <functional>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

size_t RouteHash::operator()(const std::pair<RequestMethod, std::string>& pair) const  {
    size_t h1 = std::hash<int>{}(static_cast<int>(pair.first));
    size_t h2 = std::hash<std::string>{}(pair.second);
    return h1 ^ (h2 << 1);
}

RoutePattern::RoutePattern(const std::string& pattern, std::function<HttpResponse(const HttpRequest&)>& handler): original_pattern(pattern), handler(handler) {
    parse_pattern(pattern);
}

void RoutePattern::parse_pattern(const std::string& pattern) {
    std::istringstream ss(pattern);
    std::string segment;
    while(std::getline(ss, segment, '/')) {
        if(segment.empty()) continue;
        segments.push_back(segment);
        if(is_param_segment(segment)) {
            param_names.push_back(extract_param_name(segment));
        }
    }
}

bool RoutePattern::is_param_segment(const std::string& segment) const {
    return segment.size() >= 3 && segment.front() == '{' && segment.back() == '}';
}

std::string RoutePattern::extract_param_name(const std::string& segment) const {
    return segment.substr(1, segment.size() - 2);
}

bool RoutePattern::matches(const std::string& path, std::map<std::string, std::string>& params) const {
    std::vector<std::string> path_segments;
    std::istringstream ss(path);
    std::string segment;
    while(std::getline(ss, segment, '/')) {
        if(segment.empty()) continue;
        path_segments.push_back(segment);
    }
    if(path_segments.size() != segments.size()) return false;
    size_t param_index = 0;
    for(size_t i = 0; i < segments.size(); ++i) {
        if(is_param_segment(segments[i])) {
            if(param_index < segments.size()) {
                params[param_names[param_index]] = path_segments[i];
                param_index++;
            } else {
                if (segments[i] != path_segments[i]) {
                    return false;
                }
            }
        }
    }
    return true;
}

std::optional<std::function<HttpResponse(const HttpRequest&)>> Router::match_route(RequestMethod method, const std::string& path, HttpRequest& request) {
    auto exact_key = std::make_pair(method, path);
    auto exact_it = exact_routes.find(exact_key);
    if (exact_it != exact_routes.end()) {
        request.path_params.clear();
        return exact_it->second;
    }
    auto param_it = param_routes.find(method);
    if (param_it != param_routes.end()) {
        for (const auto& route_pattern : param_it->second) {
            std::map<std::string, std::string> path_params;
            if (route_pattern.matches(path, path_params)) {
                request.path_params = std::move(path_params);
                return route_pattern.handler;
            }
        }
    }
    return std::nullopt;
}

void Router::add_route(RequestMethod method, const std::string& route, std::function<HttpResponse(const HttpRequest&)> handler) {
    if(has_parameters(route)) {
        param_routes[method].emplace_back(route, handler);
    } else {
        exact_routes.emplace(std:: make_pair(method, route), handler);
    }
}

std::optional<routes_iterator> Router::get_route(route route) {
    if (route.second.find('?') != std::string::npos) {
        route.second =  route.second.substr(0, route.second.find('?'));
    }
    auto it = exact_routes.find(route);
    if (it == exact_routes.end()) {
        return std::nullopt;
    }
    return std::make_optional(it);
}

bool Router::has_parameters(const std::string& route_pattern) {
    return route_pattern.find('{') != std::string::npos && 
        route_pattern.find('}') != std::string::npos; 
}
