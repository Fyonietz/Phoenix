#include "route_register.hpp"
#include <iostream>

// DEFINITION of GlobalRoute with proper C linkage
extern "C" {
EXPORT RouteStruct GlobalRoute;
}

// Define RouteEntry structure
struct RouteEntry {
  const char *path;
  int (*handler)(mg_connection *, void *);
  void *data = nullptr;
};

// Define global route vector
static std::vector<RouteEntry> &get_routes_internal() {
  static std::vector<RouteEntry> routes_instance;
  return routes_instance;
}

void routes_register(struct mg_context *context, const char *url,
                     int (*handler)(struct mg_connection *, void *),
                     void *data) {
  if (context && handler) {
    mg_set_request_handler(context, url, handler, data);
    std::cout << Formatter_Routes::info << "Registered route: " << url
              << std::endl;
  } else {
    std::cerr << Formatter_Routes::err << "Failed to register route: " << url
              << std::endl;
  }
}

EXPORT void add_route(const char *path, int (*handler)(mg_connection *, void *),
                      void *data) {
  get_routes_internal().push_back({path, handler, data});
}

EXPORT size_t get_routes_count() { return get_routes_internal().size(); }

EXPORT void routes_update(SharedData *SD) {
  Formatter_Routes::info = "[> Phoenix [Routes] > Info]: ";
  Formatter_Routes::err = "[> Phoenix [Routes] > Error]: ";

  if (!SD || !SD->context) {
    std::cerr << Formatter_Routes::err << "Missing Context (SD=" << SD;
    if (SD)
      std::cerr << ", context=" << SD->context;
    std::cerr << ")" << std::endl;
    return;
  }

  GlobalRoute = {routes_register};
  auto &routes = get_routes_internal();

  // Register all routes
  for (const auto &route : routes) {
    if (route.path && route.handler) {
      GlobalRoute.add(SD->context, route.path, route.handler, route.data);
    } else {
      std::cerr << Formatter_Routes::err << "Invalid route configuration"
                << std::endl;
    }
  }
}
