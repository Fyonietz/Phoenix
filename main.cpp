#include <iostream>
#include <windows.h>
#include "engine.hpp"



int main(){
    initConfig();
    if(Config::routes == "debug"){
        debug_start(Config::root.c_str());
         std::cout << "Method:" << Config::routes << std::endl;
        debug();
        std::thread t(debug_watcher);
        t.join();
    }else{
        release_start(Config::root.c_str());
        std::cout << "Method:" << Config::routes << std::endl;
        release();
    }
        

    
    return 0;
}