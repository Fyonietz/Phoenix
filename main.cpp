#include <iostream>
#include "engine.hpp"

int main(){
    initConfig();
    start(Config::root.c_str());
    return 0;
}