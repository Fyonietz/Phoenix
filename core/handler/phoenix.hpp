#include "pyro.hpp"
#include <route_register.hpp>
#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif
struct mg_connection;
EXPORT int default_handler(struct mg_connection *connection,
                           void * /*callbackdata*/);
EXPORT int home(struct mg_connection *connection, void *callback);

// Even simpler - no EXPORT in macro
#define route(PATH, NAME)                                                      \
  int NAME(struct mg_connection *connection, void *cb);                        \
  namespace {                                                                  \
  struct NAME##_Reg {                                                          \
    NAME##_Reg() { add_route(PATH, NAME); }                                    \
  } NAME##_instance;                                                           \
  }                                                                            \
  int NAME(struct mg_connection *connection, void *cb)

#define ASYNC (engine_running.load() && !shutdown_requested.load())
