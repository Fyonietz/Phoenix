#include "../core/engine.hpp"
#include "register.hpp"
#include "../libs/civetweb.h"

static mg_context* server_context = nullptr;
static Router global_router;

extern "C" EXPORT void Routes_LinkUp(const char *root, const char *port, 
                                   const char *threads, const char *alive) {
    if (server_context) {
        mg_stop(server_context);
        Router_free(&global_router);
    }

    const char *config[] = {
        "document_root", root,
        "listening_ports", port,
        "num_threads", threads,
        "enable_keep_alive", alive,
        nullptr
    };

    server_context = mg_start(nullptr, nullptr, config);
    if (!server_context) {
        std::cerr << "Failed To Start Server" << std::endl;
        return;
    }

    std::cout << "Server running on http://localhost:" << port << std::endl;

    Router_init(&global_router, 10);
    Router_add(&global_router, "/times", time_handler);
    Router_register(&global_router, server_context);
}