#ifndef ENGINE_DEF
#define ENGINE_DEF

#ifdef _WIN32
    #include <windows.h>
    typedef HMODULE LibraryHandle;
    #define LoadLibrary(path) LoadLibraryW(path)
    #define GetSymbol GetProcAddress
    #define CloseLibrary FreeLibrary
#else
    #include <dlfcn.h>
#endif

#include <string>
#include <unordered_map>
#include "civetweb.h"
#include "shared.hpp"

#ifdef _WIN32
    #define EXPORT __declspec(dllexport)
#else   
    #define EXPORT __attribute__((visibility("default")))
#endif

struct Config {
    inline static std::string root;
    inline static std::string port;
    inline static std::string threads;
    inline static std::string keep_alive;
    inline static struct mg_callbacks callbacks;
    inline static struct mg_context *context;
    static const char *server_config[];
};

struct Formatter {
    inline static std::string err;    
    inline static std::string info;    
};

std::unordered_map<std::string, std::string> wpc_loader(const std::string& path);
bool config_declare();
bool file_watcher();
bool library_loader_and_routes_register();
bool engine_start();
#endif // ENGINE_DEF

