#ifndef APP
#define APP

#include "starter.hpp"
//UTILITY
//Converter CharToWChar
std::wstring CharToWChar(const char* str);



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

