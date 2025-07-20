#include "handler.hpp"

EXPORT int about(struct mg_connection *connection,void *callback){
     const std::string file_path = "public/views/about.html";

    std::ifstream file(file_path);
    if (!file) {
        // File not found, return 404
        const char *msg = "404 Not Found";
        mg_printf(connection,
                  "HTTP/1.1 404 Not Found\r\n"
                  "Content-Type: text/plain\r\n"
                  "Content-Length: %zu\r\n\r\n%s",
                  strlen(msg), msg);
        return 1;
    }

    // Read the entire file into a string
    std::ostringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    // Send the HTTP headers and the HTML content
    mg_printf(connection,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: text/html\r\n"
              "Content-Length: %zu\r\n\r\n%s",
              content.length(), content.c_str());

    return 200;  
};
