#include "register.hpp"
#include "../pages/handler.hpp"
#include <iostream>

void Route::add(struct mg_context *context,const char *url,int (*handler)(struct mg_connection *connection,void *callbackdata),void *data){
    mg_set_request_handler(context,url,handler,data);
};
void update() {
    if (!Config::context) {
        std::cerr << "Cannot register routes - server context is missing!" << std::endl;
        return;
    }
    
    // Register all routes here
    Route.add(Config::context, "/hello", hello, 0);
    std::cout << "\rRoutes registered successfully" << std::flush;
}   
