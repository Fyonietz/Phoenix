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
#include <windows.h>
#include "civetweb.h"
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
};
//Route Typedef 
typedef void(*Route)();
//Route Register
void routes();
//Server Start
void start(const char *root,const char *port="9000",const char *threads="64",const char *alive="yes");
#endif // !APP

