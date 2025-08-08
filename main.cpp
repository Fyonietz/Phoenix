#include <iostream>
#include "engine.hpp"



int main() {
    initConfig();
    
    if (Config::routes == "debug") {
        debug_start(Config::root.c_str());
        std::cout << Global::info << "Method:" << Config::routes << std::endl;
        
        // Wait briefly to ensure server is fully initialized
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        if (debug()) {
            std::thread t(debug_watcher);
            t.detach(); // Use detach instead of join to keep running
        }
    } else {
        release_start(Config::root.c_str());
        std::cout <<Global::info<< "Method:" << Config::routes << std::endl;
        
        // Wait briefly to ensure server is fully initialized
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        release();
    }
    
 // Keep main thread alive
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        // Optionally, break on signal or condition
    }

    cleanup_temp_so_files(); // Call cleanup before exit
    return 0;
}