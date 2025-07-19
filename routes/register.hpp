#ifndef ROUTER
#define ROUTER

#pragma once
#include "civetweb.h"
#include "../pages/handler.hpp"
#include <iostream>

#ifdef _WIN32
    #define EXPORT __declspec(dllexport)
#else
    #define EXPORT
#endif

extern "C" {
    typedef void(*UpdateFunc)();

    typedef struct {
        const char *url;
        mg_request_handler handler;
    } Route;

    typedef struct {
        Route *routes;
        size_t count;
        size_t capacity;
    } Router;

    EXPORT void Router_init(Router *router, size_t initial_capacity);
    EXPORT void Router_add(Router *router, const char *url, mg_request_handler handler);
    EXPORT void Router_register(Router *router, struct mg_context *context);
    EXPORT void Router_free(Router *router);

    // Just a declaration here, no function body in header!
    EXPORT void Routes_LinkUp(const char *root, const char *port = "9000", const char *threads = "64", const char *alive = "yes");
}

#endif // ROUTER
