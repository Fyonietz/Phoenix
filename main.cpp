#include <iostream>
#include <windows.h>
#include "engine.hpp"



int main(){
    initConfig();
    routes();
    start(Config::root.c_str());
    return 0;
}