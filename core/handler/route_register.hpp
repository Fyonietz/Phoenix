#ifndef ROUTES
#define ROUTES
#ifdef _WIN32
    #define EXPORT __declspec(dllexport)
#else   
    #define EXPORT __attribute__((visibility("default")))
#endif
#include "civetweb.h"
#include <string>
#include <vector>
#include <cstring>

struct SharedData {
    struct mg_context* context;
    struct mg_callbacks callbacks;
    
    SharedData() : context(nullptr) {
        memset(&callbacks, 0, sizeof(callbacks));
    }
    
    SharedData(const SharedData&) = delete;
    SharedData& operator=(const SharedData&) = delete;
};

struct Formatter_Routes{
  inline static std::string info;
  inline static std::string err;
};

typedef struct {
    void (*add)(struct mg_context*, const char*, int (*)(struct mg_connection*, void*), void*);
} RouteStruct;

// Function declarations
EXPORT void add_route(const char* path, int (*handler)(struct mg_connection*, void*), void* data = nullptr);
EXPORT void routes_register(struct mg_context* context, const char* path, 
                          int (*handler)(struct mg_connection*, void*), void* data);
EXPORT size_t get_routes_count();

// Update function for registering routes
extern "C" {
    EXPORT void routes_update(SharedData *SD);
}

// DECLARATION only (not definition)
extern "C" {
    EXPORT extern RouteStruct GlobalRoute;  // Note: added 'extern' keyword
}

EXPORT void routes();
#endif // !ROUTES
