#include "../core/starter.hpp"
#ifdef _WIN32
    #define EXPORT __declspec(dllexport)
#else   
    #define EXPORT
#endif
EXPORT int about(struct mg_connection *connection,void *callback);