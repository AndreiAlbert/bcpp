# BCPP - High-Performance C++ HTTP Server
A lightweight, multithreaded HTTP/1.1 server implementation written C++20, featuring request routing and connection pooling.

## Features
- **HTTP/1.1 Protocol Support**: Full implementation with persistent connections (Keep-Alive)
- **Multithreaded Architecture**: Thread pool-based request handling for high concurrency
- **Request Routing**: Flexible routing system supporting multiple HTTP methods (GET, POST, PUT, DELETE, HEAD, OPTIONS)
- **Query Parameter Parsing**: Automatic extraction and parsing of URL query parameters
- **MIME Type Support**: Built-in support for common content types (JSON, HTML, CSS, images, etc.)
- **Connection Management**: Configurable Keep-Alive timeout and maximum request limits

## Architecture

The server is built with a modular architecture:

- **HttpServer**: Main server class handling socket operations and connection management
- **Router**: Request routing and handler registration system
- **HttpRequestParser**: HTTP request parsing with header and body extraction
- **HttpResponse**: Response building with status codes and headers
- **ThreadPool**: Fixed-size thread pool for handling concurrent requests
- **Logger**: Thread-safe logging system with configurable levels

## Prerequisites

- C++20 compatible compiler (Clang++ recommended)
- CMake 3.10 or higher
- POSIX-compliant system (Linux/macOS)

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
    
    // Start the server
    server.start();
    return 0;
}
```

### Advanced Configuration

```cpp
HttpServer server(8080, 8); // 8 worker threads
server.set_keep_alive_timeout(60); // 60 seconds
server.set_max_requests(200); // Max 200 requests per connection

// Add routes for different HTTP methods
server.router.add_route(RequestMethod::POST, "/api/users", create_user_handler);
server.router.add_route(RequestMethod::GET, "/api/users", get_users_handler);
server.router.add_route(RequestMethod::DELETE, "/api/users", delete_user_handler);
```

### Request Handler Example

```cpp
HttpResponse create_user_handler(const HttpRequest& request) {
    HttpResponse response;
    
    // Access request body
    std::string body = request.body;
    
    // Access query parameters
    auto name_param = request.query_params.find("name");
    if (name_param != request.query_params.end()) {
        // Process name parameter
    }
    
    // Access headers
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

### Keep-Alive Settings
- **Timeout**: Configure how long connections stay open (default: 30 seconds)
- **Max Requests**: Maximum requests per connection (default: 100)

### Thread Pool
- **Thread Count**: Number of worker threads (default: hardware concurrency)

## Performance Features

- **Connection Pooling**: Reuse TCP connections for multiple requests
- **Thread Pool**: Fixed thread pool prevents thread creation overhead
- **Efficient Parsing**: Optimized HTTP request parsing with minimal memory allocation
- **Signal Handling**: Graceful shutdown without dropping active connections

## Testing

```bash
# Start the server
./bcpp

# Test with curl
curl -X GET http://localhost:8080/hello
curl -X POST http://localhost:8080/api/data -d '{"test": "data"}'
curl -X GET "http://localhost:8080/search?q=test&limit=10"
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

## License
If someone wants to use this for some reason please go ahead.
