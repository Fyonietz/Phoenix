#include "register.hpp"
#include <iostream>

void init(){
    std::cout << "Plugin Started" << std::endl;
}

void update(){
    std::cout << "Updated coy" << std::endl;
}

void shutdown(){
    std::cout << "Plugin Shutdown!" << std::endl;
}
