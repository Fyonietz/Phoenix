#include "shared.hpp"

static struct mg_context* server_context = nullptr;
static bool server_running = false;

void set_server_context(struct mg_context* ctx) {
    server_context = ctx;
}

struct mg_context* get_server_context() {
    return server_context;
}

void set_server_running(bool running) {
    server_running = running;
}

bool get_server_running() {
    return server_running;
}