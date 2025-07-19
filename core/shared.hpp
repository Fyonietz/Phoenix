#ifndef SHARED_HPP
#define SHARED_HPP

struct mg_context;

#ifdef __cplusplus
extern "C" {
#endif

void set_server_context(struct mg_context* ctx);
struct mg_context* get_server_context();
void set_server_running(bool running);
bool get_server_running();

#ifdef __cplusplus
}
#endif

#endif