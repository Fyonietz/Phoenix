#include "register.hpp"
#include "../pages/handler.hpp"
#include <iostream>
std::string info = "Phoenix[Info]: ";
void Route::add(struct mg_context *context,const char *url,int (*handler)(struct mg_connection *connection,void *callbackdata),void *data){
    mg_set_request_handler(context,url,handler,data);
};
extern "C" EXPORT void update(struct mg_context *context) {
    if (!context) {
        std::cerr <<info<< "Cannot register routes - server context is missing!" << std::endl;
        return;
    }
    
    // Register all routes here
    Route.add(context, "/api/hello", hello, 0);
    Route.add(context, "/api/other", hello_other, 0);
    std::cout <<info << "Routes registered successfully" << std::flush <<std::endl;
}   
