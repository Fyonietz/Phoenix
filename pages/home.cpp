#include "handler.hpp"


int time_handler(struct mg_connection *connection,void *cbdata){
        mg_printf(connection,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: text/plain\r\n"
              "Connection: close\r\n\r\n"
              "Time infos\n");
    return 1;
}