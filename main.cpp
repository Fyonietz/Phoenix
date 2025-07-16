#include <iostream>
#include "app.hpp"

int main(){
    initConfig();
    start(Config::root.c_str());
    return 0;
}