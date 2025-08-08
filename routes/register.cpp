#include "register.hpp"
#include "../pages/handler.hpp"
#include <iostream>

std::string info = " > Phoenix[Info]: ";
void Route::add(struct mg_context *context,const char *url,int (*handler)(struct mg_connection *connection,void *callbackdata),void *data){
    mg_set_request_handler(context,url,handler,data);
};
extern "C" EXPORT void update(struct mg_context *context) {
    if (!context) {
        std::cerr <<info<< "Cannot Register Routes - Server Context Is Missing!" << std::endl;
        return;
    }
    std::cout <<info << "Routes Check" << std::endl <<std::flush;
    // Register all routes here
    Route.add(context, "/about", about, 0);
    Route.add(context,"/",home,0);
    Route.add(context, "/example", [](struct mg_connection *conn, void *data) -> int {
        mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n");
        mg_printf(conn, "Hello, World!");
        return 200;
    });
    std::cout <<info << "Routes Safe" << std::flush <<std::endl;
}
