#include "handler.hpp"
Pnix Server;


EXPORT int home(struct mg_connection *connection,void *callback){
    Server.home("public/layout.html",connection);
    return 200;  
};

EXPORT int about(struct mg_connection *connection,void *callback){
    Server.SSR("public/views/about.html",connection);
    return 200;  
};
