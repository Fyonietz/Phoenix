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
#include "shared.hpp"


std::wstring CharToWChar(const char* str);

std::unordered_map<std::string, std::string> loadConfig();
void initConfig();

struct Config {
    inline static std::string root;
    inline static std::string port;
    inline static std::string threads;
    inline static std::string keep_alive;
    inline static std::string routes;
};

struct Global {
    static std::string dll_name;
};

typedef void (*RouteFunc)();
typedef void (*RoutesLinkUpFunc)(const char* root, const char* port, const char* threads, const char* alive);

void static_routes();
bool dynamic_routes();
void dynamic_routes_starter();
void start();

#endif // APP