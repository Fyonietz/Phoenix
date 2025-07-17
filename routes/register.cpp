#include "register.hpp"
#include <iostream>

void init(){
    std::cout << "Plugin Started" << std::endl;
}

void update(){
    std::cout << "Updateds" << std::endl;
}

void shutdown(){
    std::cout << "Plugin Shutdown!" << std::endl;
}
