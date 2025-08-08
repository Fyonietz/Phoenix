#include "../core/starter.hpp"
#include "pyro.hpp"
#ifdef _WIN32
    #define EXPORT __declspec(dllexport)
#else   
    #define EXPORT __attribute__((visibility("default")))
#endif
EXPORT int about(struct mg_connection *connection,void *callback);
EXPORT int home(struct mg_connection *connection,void *callback);
EXPORT int homes(struct mg_connection *connection,void *callback);