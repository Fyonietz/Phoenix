#pragma once
#include "../core/starter.hpp"
#include "../libs/civetweb.h"

#ifndef EXPORT
#define EXPORT __attribute__((visibility("default")))
#endif

extern "C" {
    typedef void(*UpdateFunc)(struct mg_context *);
    EXPORT void update(struct mg_context *ctx);
    EXPORT struct Route {
        void add(struct mg_context *context, const char *url, int (*handler)(struct mg_connection *connection, void *callbackdata), void *data = 0);
    } Route;
}