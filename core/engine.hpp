#ifndef APP
#define APP

#include "starter.hpp"
#include <mutex>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
typedef HMODULE LibraryHandle;
#define LoadLibrary(path) LoadLibraryW(path)
#define GetSymbol GetProcAddress
#define CloseLibrary FreeLibrary
#else
#include <dlfcn.h>
typedef void* LibraryHandle;
#define LoadLibrary(path) dlopen(path, RTLD_LAZY)
#define GetSymbol dlsym
#define CloseLibrary dlclose
#endif

typedef void(*RouteFunc)();
extern LibraryHandle dllHandle;
extern RouteFunc routefunc;
extern std::filesystem::file_time_type lastWriteTime;
extern std::mutex dllMutex;

bool debug();
void debug_watcher();
void debug_start(const char *root, const char *port = "9000", const char *threads = "64", const char *alive = "yes");
void release_start(const char *root, const char *port = "9000", const char *threads = "64", const char *alive = "yes");
void release();
void cleanup_temp_so_files();
#endif // !APP

