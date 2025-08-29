# BCPP - High-Performance C++ HTTP Server
A lightweight, multithreaded HTTP/1.1 server implementation written in C++20, featuring non-blocking I/O, request routing, query parameter parsing, and connection pooling.

## Features
- **HTTP/1.1 Protocol Support**: Full implementation with persistent connections (Keep-Alive)
- **Non-blocking I/O**: Epoll-based event-driven architecture with edge-triggered mode for high-performance concurrent request handling
- **Multithreaded Architecture**: Multiple event loops running on separate threads for optimal CPU utilization
- **Request Routing**: Flexible routing system supporting multiple HTTP methods (GET, POST, PUT, DELETE, HEAD, OPTIONS)
- **Path Parameters**: Dynamic route segments with parameter extraction (`/users/{id}`, `/api/v1/{resource}/{action}`)
- **Query Parameter Parsing**: Automatic extraction, parsing, and URL decoding of query parameters
- **MIME Type Support**: Built-in support for common content types (JSON, HTML, CSS, images, etc.)
- **Connection Management**: Configurable Keep-Alive timeout and connection pooling with automatic cleanup

## Architecture

The server is built with a high-performance, event-driven architecture:

- **HttpServer**: Main server class handling socket operations and load balancing across event loops
- **EventLoop**: Non-blocking I/O event loops using Linux epoll for efficient connection management
- **Connection**: Individual connection state management with read/write buffers and timeout tracking
- **Router**: Request routing system with support for both exact matches and parameterized routes
- **HttpRequestParser**: Streaming HTTP request parser with header, body, and query parameter extraction
- **HttpResponse**: Response builder with status codes, headers, and automatic content-length calculation
- **Logger**: Thread-safe logging system with configurable levels and timestamps

## Performance Features

- **Non-blocking I/O**: Uses Linux epoll with edge-triggered mode for maximum throughput
- **Zero-copy Operations**: Minimal memory allocations and efficient buffer management
- **Connection Pooling**: Persistent HTTP/1.1 connections reduce overhead
- **Load Balancing**: Round-robin distribution of connections across worker threads
- **Timeout Management**: Automatic cleanup of idle connections
- **Signal Handling**: Graceful shutdown without dropping active connections

## Prerequisites

- C++20 compatible compiler (Clang++ recommended)
- CMake 3.10 or higher
- Linux system (uses epoll - POSIX-compliant)

## Building

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

### Basic Server Setup

```cpp
#include "include/http_server.hpp"

int main() {
    // Create server on port 8080 with default thread count
    HttpServer server(8080);
    
    // Add a simple GET route
    server.router.add_route(RequestMethod::GET, "/hello", 
        [](const HttpRequest& request) {
            HttpResponse response;
            response.set_body("{\"message\": \"Hello, World!\"}");
            response.set_content_type(MimeType::ApplicationJson);
            response.set_status(HttpStatusCode::OK);
            return response;
        });
    
    // Start the server (non-blocking event loops)
    server.start();
    return 0;
}
```

### Advanced Configuration

```cpp
HttpServer server(8080, 8); // 8 worker event loops
server.set_keep_alive_timeout(60); // 60 seconds Keep-Alive timeout

// Add routes for different HTTP methods
server.router.add_route(RequestMethod::POST, "/api/users", create_user_handler);
server.router.add_route(RequestMethod::GET, "/api/users", get_users_handler);
server.router.add_route(RequestMethod::DELETE, "/api/users/{id}", delete_user_handler);
```

### Path Parameters Example

```cpp
server.router.add_route(RequestMethod::GET, "/users/{id}/posts/{post_id}", 
    [](const HttpRequest& request) {
        HttpResponse response;
        
        // Extract path parameters
        std::string user_id = request.path_params.at("id");
        std::string post_id = request.path_params.at("post_id");
        
        std::string body = std::format(
            "{{\"user_id\": \"{}\", \"post_id\": \"{}\"}}",
            user_id, post_id
        );
        
        response.set_body(body);
        response.set_content_type(MimeType::ApplicationJson);
        response.set_status(HttpStatusCode::OK);
        return response;
    });
```

### Query Parameters Example

```cpp
server.router.add_route(RequestMethod::GET, "/search", 
    [](const HttpRequest& request) {
        HttpResponse response;
        
        // Extract query parameters with defaults
        auto query_it = request.query_params.find("q");
        auto limit_it = request.query_params.find("limit");
        auto sort_it = request.query_params.find("sort");
        
        std::string query = query_it != request.query_params.end() ? 
                           query_it->second : "";
        std::string limit = limit_it != request.query_params.end() ? 
                           limit_it->second : "10";
        std::string sort = sort_it != request.query_params.end() ? 
                          sort_it->second : "relevance";
        
        // URL decoding is handled automatically
        std::string body = std::format(
            "{{\"query\": \"{}\", \"limit\": {}, \"sort\": \"{}\"}}",
            query, limit, sort
        );
        
        response.set_body(body);
        response.set_content_type(MimeType::ApplicationJson);
        response.set_status(HttpStatusCode::OK);
        return response;
    });
```

### Complete Request Handler Example

```cpp
HttpResponse create_user_handler(const HttpRequest& request) {
    HttpResponse response;
    
    // Access request body
    std::string body = request.body;
    
    // Access query parameters using convenience methods
    auto name = request.get_query_param("name");
    if (name.has_value()) {
        std::string name_value = name.value(); // Already URL decoded
    }
    
    // Access path parameters with safety
    auto user_id = request.get_path_param("id");
    if (user_id.has_value()) {
        std::string id = user_id.value();
    }
    
    // Access headers (case-sensitive)
    auto content_type = request.headers.find("Content-Type");
    
    // Build response
    response.set_status(HttpStatusCode::Created);
    response.set_content_type(MimeType::ApplicationJson);
    response.set_body("{\"status\": \"created\"}");
    
    return response;
}
```

## HTTP Methods Supported

- `GET` - Retrieve resources
- `POST` - Create resources  
- `PUT` - Update resources
- `DELETE` - Delete resources
- `HEAD` - Get headers only
- `OPTIONS` - Get allowed methods

## Status Codes Supported

- 1xx: Informational (Continue)
- 2xx: Success (OK, Created, Accepted, No Content)
- 3xx: Redirection (Moved Permanently, Found)
- 4xx: Client Errors (Bad Request, Unauthorized, Forbidden, Not Found)
- 5xx: Server Errors (Internal Server Error, Not Implemented)

## MIME Types

Built-in support for:
- Text formats: `text/plain`, `text/html`, `text/css`, `text/javascript`
- Application formats: `application/json`, `application/xml`, `application/pdf`
- Image formats: `image/jpeg`, `image/png`, `image/gif`
- Media formats: `audio/mpeg`, `video/mp4`
- Form data: `multipart/form-data`

## Query Parameter Features

- **Automatic URL Decoding**: Handles percent-encoding (`%20` → space) and plus-encoding (`+` → space)
- **Key-Value Parsing**: Supports standard `key=value` format
- **Empty Values**: Handles parameters without values (`?debug&verbose=true`)
- **Special Characters**: Properly decodes international characters and symbols
- **Error Handling**: Graceful handling of malformed encoding

## Logging

The server includes a comprehensive logging system:

```cpp
Logger& logger = Logger::get_instance();
logger.set_level(LogLevel::INFO);
logger.info("Server started successfully");
logger.error("Failed to process request");
```

Log levels: `DEBUG`, `INFO`, `WARNING`, `ERROR`, `CRITICAL`

## Configuration Options

### Non-blocking I/O Settings
- **Event Loops**: Number of worker event loops (default: hardware concurrency)
- **Epoll Mode**: Edge-triggered mode for maximum performance
- **Connection Limits**: Automatic management of connection lifecycle

### Keep-Alive Settings  
- **Timeout**: Configure how long connections stay open (default: 30 seconds)
- **Connection Reuse**: HTTP/1.1 persistent connections reduce TCP overhead

### Performance Tuning
- **Buffer Sizes**: Configurable read/write buffer sizes
- **Worker Threads**: Number of event loops for load distribution

## Performance Characteristics

- **High Concurrency**: Handles thousands of concurrent connections with minimal threads
- **Low Latency**: Non-blocking I/O reduces request processing time
- **Memory Efficient**: Connection pooling and efficient buffer management
- **CPU Scalable**: Multi-threaded architecture utilizes all available cores

## Testing

```bash
# Start the server
./bcpp

# Test basic routes
curl -X GET http://localhost:8080/hello

# Test path parameters
curl -X GET http://localhost:8080/users/123/posts/456

# Test query parameters
curl -X GET "http://localhost:8080/search?q=test&limit=10&sort=date"

# Test URL encoding
curl -X GET "http://localhost:8080/search?q=hello%20world&category=news%21"

# Test complex parameters
curl -X GET "http://localhost:8080/api/data?filter=active&sort=name&page=1&limit=50"

# Load testing with multiple concurrent connections
ab -n 10000 -c 100 http://localhost:8080/hello
```
## License
If someone wants to use this for some reason please go ahead.
