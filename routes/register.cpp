#include "register.hpp"
#include <cstdlib>  // for malloc, realloc, free

void Router_init(Router *router, size_t initial_capacity) {
    router->routes = (Route*) malloc(initial_capacity * sizeof(Route));
    router->count = 0;
    router->capacity = initial_capacity;
}

void Router_add(Router *router, const char *url, mg_request_handler handler) {
    if (router->count >= router->capacity) {
        router->capacity *= 2;
        router->routes = (Route*) realloc(router->routes, router->capacity * sizeof(Route));
    }
    router->routes[router->count].url = url;
    router->routes[router->count].handler = handler;
    router->count++;
}

void Router_register(Router *router, struct mg_context *context) {
    for (size_t i = 0; i < router->count; ++i) {
        mg_set_request_handler(context, router->routes[i].url, router->routes[i].handler, 0);
    }
}

void Router_free(Router *router) {
    free(router->routes);
    router->routes = NULL;
    router->count = 0;
    router->capacity = 0;
}
