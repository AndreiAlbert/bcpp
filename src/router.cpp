#include "../include/router.hpp"
#include <functional>
#include <optional>
#include <unordered_map>
#include <utility>

size_t RouteHash::operator()(const std::pair<RequestMethod, std::string>& pair) const  {
    size_t h1 = std::hash<int>{}(static_cast<int>(pair.first));
    size_t h2 = std::hash<std::string>{}(pair.second);
    return h1 ^ (h2 << 1);
}

void Router::add_route(RequestMethod method, const std::string& route, std::function<std::string(const HttpRequest&)> handler) {
    routes.emplace(std::make_pair(method, route), handler);
}

std::optional<routes_iterator> Router::get_route(route route) {
    auto it = routes.find(route);
    if (it == routes.end()) {
        return std::nullopt;
    }
    return std::make_optional(it);
}
