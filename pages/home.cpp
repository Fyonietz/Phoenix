#include "handler.hpp"

int hello(struct mg_connection *connection,void *callback){
     const std::string response = "User info from /user!";
    mg_printf(connection,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: text/plain\r\n"
              "Content-Length: %zu\r\n\r\n%s",
              response.length(), response.c_str());
    return 1;  // Indicate the request was handled
};

