#ifndef STARTER_APP
#define STARTER_APP
#pragma once
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

//Config Loader
std::unordered_map<std::string,std::string> loadConfig();

//Init 
void initConfig();
void updateConfig();
//Config Struct
struct Config{
    inline static std::string root;
    inline static std::string port;
    inline static std::string threads;
    inline static std::string keep_alive;
    inline static std::string routes;
    inline static struct mg_callbacks callbacks;
    inline static struct mg_context *context;
    static const char *server_config[];
};

struct Global{
       static std::string dll_name;
       static std::string info;
};
#endif // !STARTER_APP


