#include "handler.hpp"

EXPORT int hello(struct mg_connection *connection,void *callback){
     const std::string response = "User info from /hello!";
    mg_printf(connection,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: text/plain\r\n"
              "Content-Length: %zu\r\n\r\n%s",
              response.length(), response.c_str());
    return 200;  // Indicate the request was handled
};

EXPORT int hello_other(struct mg_connection *connection,void *callback){
     const std::string response = "User info from /hello_other oi";
    mg_printf(connection,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: text/plain\r\n"
              "Content-Length: %zu\r\n\r\n%s",
              response.length(), response.c_str());
    return 200;  // Indicate the request was handled
};