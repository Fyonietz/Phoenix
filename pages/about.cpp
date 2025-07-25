#include "handler.hpp"
Pnix Server;


EXPORT int about(struct mg_connection *connection,void *callback){
    Server.serve("public/views/about.html",connection);
    return 200;  
};
