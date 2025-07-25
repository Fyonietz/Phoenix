#include "../core/starter.hpp"
#ifdef _WIN32
    #define EXPORT __declspec(dllexport)
#else   
    #define EXPORT
#endif
extern "C" struct EXPORT Pnix{
    void serve(const std::string &path,struct mg_connection *connection){

    std::ifstream file(path);
    if (!file) {
        // File not found, return 404
        const char *msg = "404 Not Found";
        mg_printf(connection,
                  "HTTP/1.1 404 Not Found\r\n"
                  "Content-Type: text/plain\r\n"
                  "Content-Length: %zu\r\n\r\n%s",
                  strlen(msg), msg);
        return;
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
    }
};

EXPORT int about(struct mg_connection *connection,void *callback);
EXPORT int patir(struct mg_connection *connection,void *callback);