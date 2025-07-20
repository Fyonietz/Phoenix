#ifndef APP
#define APP
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unordered_map>
#include <filesystem>
#include <thread>
#include <chrono>
#include <windows.h>
#include "civetweb.h"
#include <thread>
#include <mutex>
//UTILITY
//Converter CharToWChar
std::wstring CharToWChar(const char* str);

//Config Loader
std::unordered_map<std::string,std::string> loadConfig();
//Init 
void initConfig();
//Config Struct
struct Config{
    inline static std::string root;
    inline static std::string port;
    inline static std::string threads;
    inline static std::string keep_alive;
    inline static std::string routes;
};

struct Global{
       static std::string dll_name;
};
//Route Typedef 
typedef void(*RouteFunc)();
//Route Register
void release();
//Dynamic Register
extern HMODULE dllHandle ;
extern RouteFunc routefunc ;
// extern void(*RouteFunc)();
extern std::filesystem::file_time_type lastWriteTime;
extern std::mutex dllMutex; 
bool debug();
void debug_watcher();
//Server Start
void debug_start(const char *root,const char *port="9000",const char *threads="64",const char *alive="yes");
void release_start(const char *root,const char *port="9000",const char *threads="64",const char *alive="yes");
#endif // !APP

