#include <iostream>
#include <windows.h>
#include "engine.hpp"



int main(){
    initConfig();
    start(Config::root.c_str());
    if(Config::routes == "dynamic"){
        dynamic_routes();
        std::thread t(dynamic_routes_starter);
        t.join();
    }else{
        static_routes();
    }
        std::cout << "Method:" << Config::routes << std::endl;

    
    return 0;
}