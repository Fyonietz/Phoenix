#include <iostream>
#include <windows.h>
#include <thread>
#include "engine.hpp"

int main() {
    std::cout << "Web++[Info]: Initializing Config..." << std::endl;
    initConfig();

    if (Config::routes == "dynamic") {
        std::cout << "Web++[Info]: Setting up dynamic routes..." << std::endl;
        dynamic_routes();
        dynamic_routes_starter();
    } else {
        static_routes();
    }

    std::cout << "Web++[Info]: Starting server..." << std::endl;
    start(); 

    return 0;
}
