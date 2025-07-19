#include "register.hpp"
#include <iostream>

void init(){
    std::cout << "Plugin Started" << std::endl;
}

void update(){
    std::cout << "\rUpdateds" << std::flush;
}

void shutdown(){
    std::cout << "Plugin Shutdown!" << std::endl;
}
